#include <unistd.h>
#include <string.h>
#include "connection.hpp"
#include "log.hpp"
#ifdef __linux__
#include <sys/syscall.h>
#endif
#include "public.hpp"
Connection::Connection(std::shared_ptr<EventLoop> loop, int fd, InetAddress *clientaddr) : loop_(loop), disconnect_(false)
{
    clientsock_ = std::make_unique<Socket>(fd);
    clientsock_->setNonBlocking(true); // 在边缘触发模式下必须将clintsock设为非阻塞模式
    clientsock_->setIp(clientaddr->ip());
    clientsock_->setPort(clientaddr->port());
    clientchannel_ = std::make_unique<Channel>(loop_, clientsock_->getFd());
    clientchannel_->setETMode(); // 一定要在start_monitor_read()之前调用设置边缘触发的方法
    clientchannel_->setReadCallBack(std::bind(&Connection::readCallBack, this));
    clientchannel_->setCloseCallBack(std::bind(&Connection::closeCallBack, this));
    clientchannel_->setErrorCallBack(std::bind(&Connection::errorCallBack, this));
    clientchannel_->setWriteCallBack(std::bind(&Connection::writeCallBack, this));
    clientchannel_->registerReadEvent(); // 加入epoll的监视，开始监视这个channel的可读事件
}
Connection::~Connection()
{
    std::string ip;
    in_port_t port;
    ip = clientsock_->getIP();
    port = clientsock_->getPort();
    logger.logMessage(DEBUG, __FILE__, __LINE__, "连接到(%s:%d)的Connection对象的生命周期已结束", ip.c_str(), port);
}
int Connection::getFd() const
{
    return clientsock_->getFd();
}
std::string Connection::getIP() const
{
    return clientsock_->getIP();
}
in_port_t Connection::getPort() const
{
    return clientsock_->getPort();
}

void Connection::closeCallBack()
{
    disconnect_ = true;
    clientchannel_->removeSelfFromLoop();
    closeCallBack_(shared_from_this());
}

void Connection::errorCallBack()
{
    disconnect_ = true;
    clientchannel_->removeSelfFromLoop();
    errorCallBack_(shared_from_this());
}

void Connection::setCloseCallBack(std::function<void(SharedConnectionPointer)> closeCallBack)
{
    closeCallBack_ = closeCallBack;
}

void Connection::setErrorCallBack(std::function<void(SharedConnectionPointer)> errorCallBack)
{
    errorCallBack_ = errorCallBack;
}

void Connection::readCallBack()
{
    char buffer[1024];
    while (true)
    {
        bzero(&buffer, sizeof(buffer));
        ssize_t nread = read(getFd(), buffer, sizeof(buffer));
        logger.logMessage(DEBUG, __FILE__, __LINE__, "一次read调用完成，实际读取到了%d个字节", nread);
        if (nread > 0)
        {
            logger.logMessage(DEBUG, __FILE__, __LINE__, "此次read调用尚未从底层输入缓冲区中读取所有数据，现将本次读取的数据放入inputBuffer_");
            inputBuffer_.append(buffer, nread);
        }
        else if (nread == -1 && errno == EINTR)
        {
            // 读取数据的时候被信号中断，继续读取。
            logger.logMessage(NORMAL, __FILE__, __LINE__, "读取数据的时候被信号中断，继续读取。");
            continue;
        }
        else if (nread == -1 && ((errno == EAGAIN) || (errno == EWOULDBLOCK)))
        {
            logger.logMessage(DEBUG, __FILE__, __LINE__, "底层输入缓冲区读到头了");

            while (true)
            {
                // 可以把以下代码封装在Buffer类中，还可以支持固定长度、指定报文长度和分隔符等多种格式。
                int len;
                memcpy(&len, inputBuffer_.getData(), 4); // 从inputbuffer中获取报文头部。
                // 在这里判断一下len是否是合法的数字，而不是其他非法字符，以防止发送方发送的数据不符合协议导致服务端崩溃

                if (len <= 0)
                {
                    // 既然报文没加头部，直接把inputBuffer_中的数据全部取出来，然后清空inputBuffer_
                    std::string message(inputBuffer_.getData(), inputBuffer_.getSize());
                    inputBuffer_.erase(0, inputBuffer_.getSize());
                    processCallBack_(shared_from_this(), message);
                }
                else
                {
                    // 如果inputbuffer中的数据量小于报文头部长度和数据量长度之和，说明inputbuffer中的报文内容不完整，需要退出循环，继续等到下一次读取到数据。
                    if (inputBuffer_.getSize() < len + 4)
                    {
                        break;
                    }
                    std::string message(inputBuffer_.getData() + 4, len); // 从inputbuffer中获取一个整个报文中的数据部分。
                    inputBuffer_.erase(0, len + 4);                       // 从inputbuffer中删除刚才读取的报文。
                    logger.logMessage(NORMAL, __FILE__, __LINE__, "thread %d recv from client(fd=%d,ip=%s,port=%u):%s", get_tid(), getFd(), getIP().c_str(), getPort(), message.c_str());
                    processCallBack_(shared_from_this(), message); // processCallBack()是TCPServer对于客户端数据的处理回调函数
                }
            }
            break;
        }
        else if (nread == 0)
        {
            closeCallBack();
            break;
        }
        else
        {
            errorCallBack();
            break;
        }
    }
}

void Connection::writeCallBack()
{
    // 把outputBuffer_中的数据发送出去
    logger.logMessage(DEBUG, __FILE__, __LINE__, "即将调用底层send函数，当前时间点为%s", getCurrentTimeInNanoseconds().c_str());
    int writen = ::send(getFd(), outputBuffer_.getData(), outputBuffer_.getSize(), 0);
    logger.logMessage(DEBUG, __FILE__, __LINE__, "完成调用底层send将数据发出，当前时间点为%s，当前线程id为%d", getCurrentTimeInNanoseconds().c_str(), get_tid());
    if (writen > 0)
        outputBuffer_.erase(0, writen);
    // 这里还要判断发送缓冲区中是否还有数据，如果没有数据了，则不应该再关注写事件
    if (outputBuffer_.getSize() == 0)
    {
        clientchannel_->unregisterWriteEvent();
        sendCompleteCallBack_(shared_from_this());
    }
}

void Connection::setProcessCallBack(std::function<void(SharedConnectionPointer, std::string &)> processCallBack)
{
    processCallBack_ = processCallBack;
}
void Connection::send(std::string data)
{
    // 这段代码如果由工作线程执行，工作线程将处理之后的数据放到自定义缓冲区中，然后下次写事件就绪之后，才发送出去
    // 这段代码也可能由从线程执行，这是在没有工作线程的情况下
    if (disconnect_ == true)
    {
        logger.logMessage(DEBUG, __FILE__, __LINE__, "客户端连接已断开，Connection::send直接返回", get_tid());
        return;
    }
    // 因为前面做了优化，导致从线程在没有工作线程的情况下会直接处理业务数据，所以如果是从线程直接处理业务数据，则处理完成之后可以安全的操作自定义输出缓冲区，因为此时不涉及多线程安全问题，如果不是从线程，就需要异步事件通知机制了
    // 所以这里判断当前线程是从线程还是工作线程
    if (loop_->isIOThread())
    {
        // loop_中记录着从线程的线程id，这个函数会比较当前线程的id是否和从线程的id相同，因为这段代码可能被工作线程执行，这样就会导致loop_记载的线程id和当前获取的线程id不一样
        // 进入这个if就代表目前这段代码是从线程在执行，从线程可以直接安全的操控自定义输出缓冲区，所以直接执行sendInIOThread函数即可
        logger.logMessage(DEBUG, __FILE__, __LINE__, "没有工作线程，从线程将直接处理之后的数据加上报头之后放入自定义输出缓冲区");
        sendInIOThread(data);
    }
    else
    {
        // 工作线程这里还要通知从线程将处理之后的数据放到自定义输出缓冲区中
        logger.logMessage(DEBUG, __FILE__, __LINE__, "当前工作线程是%d,即将把填充自定义输出缓冲区的任务放入从线程的任务队列中,当前数据为%s", get_tid(), data.c_str());
        loop_->addTaskToQueue(std::bind(&Connection::sendInIOThread, this, std::placeholders::_1), data);
    }
}
void Connection::setSendCompleteCallBack(std::function<void(SharedConnectionPointer)> sendCompleteCallBack)
{
    sendCompleteCallBack_ = sendCompleteCallBack;
}
// TODO sendInIOThread函数的参数和当初传入时的不一致，需要解决
void Connection::sendInIOThread(std::string data)
{
    logger.logMessage(DEBUG, __FILE__, __LINE__, "即将把处理之后的数据加上报头放入自定义输出缓冲区，当前时间点为%s，未加报头的数据为%s", getCurrentTimeInNanoseconds().c_str(), data.c_str());
    outputBuffer_.appendWithHead(data);
    logger.logMessage(DEBUG, __FILE__, __LINE__, "已完成将处理之后的数据加上报头并放入自定义输出缓冲区，当前时间点为%s，当前线程id为%d，当前输出输出缓冲区中的内容为%s", getCurrentTimeInNanoseconds().c_str(), get_tid(), outputBuffer_.getData());
    clientchannel_->registerWriteEvent();
}