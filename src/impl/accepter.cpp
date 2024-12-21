#include "accepter.hpp"
#include "log.hpp"
Accepter::Accepter(std::shared_ptr<EventLoop> loop, const std::string &ip, const in_port_t port) 
{
    loop_ = loop;
    servsock_ = std::make_unique<Socket>(createNonBlockingSocket());
    // 设置几个套接字选项
    servsock_->setKeepAlive(true);
    servsock_->setReuseAddr(true);
    servsock_->setReusePort(true);
    servsock_->setTcpNoDelay(true);
    // 绑定监听地址然后转换到监听模式
    InetAddress servaddr(ip, port); // 创建封装之后的地址对象，构造函数传入ip地址和端口号
    servsock_->bind(servaddr);
    servsock_->listen();
    /////////////
    acceptchannel_ = std::make_unique<Channel>(loop_, servsock_->getFd());
    acceptchannel_->setReadCallBack(std::bind(&Accepter::handleNewConnection, this)); // 设置该Channel在读事件就绪之后应该调用的回调函数
    acceptchannel_->registerReadEvent();                                              // 将该Channel设为应该监视可读事件，也就是客户端来连接的事件
}
Accepter::~Accepter()
{
    // loop_是外部传入的，不需要释放，其他指针也交给智能指针管理了，所以也不需要主动释放了
}

void Accepter::handleNewConnection()
{
    if (acceptchannel_->getRevents() & EPOLLHUP)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, "可能未调用listen函数使得监听套接字变为被动监听状态");
        exit(-1);
    }
    InetAddress clientaddr;
    int fd = servsock_->accept(clientaddr);
    acceptCallBack_(fd, clientaddr);
}

void Accepter::setAcceptCallBack(std::function<void(int, InetAddress &)> acceptCallBack)
{
    acceptCallBack_ = acceptCallBack;
}