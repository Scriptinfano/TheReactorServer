#include "business.hpp"
#include "log.hpp"
#include "public.hpp"
#include <unistd.h>
#include <iostream>
#ifdef __linux__
#include <sys/syscall.h>
#endif
EchoServer::EchoServer(const std::string &ip, in_port_t port, int threadnum, int workthreadnum)
{
    tcpserver_ = std::make_unique<TCPServer>(ip, port, threadnum);
    threadpool_ = std::make_unique<ThreadPool>(workthreadnum, "worker_thread");
    // 让TCPServer最后再回调到自己的函数中
    tcpserver_->setAcceptCallBack(std::bind(&EchoServer::acceptCallBack, this, std::placeholders::_1));
    tcpserver_->setCloseCallBack(std::bind(&EchoServer::closeCallBack, this, std::placeholders::_1));
    tcpserver_->setEpollTimeoutCallBack(std::bind(&EchoServer::epollTimeoutCallBack, this, std::placeholders::_1));
    tcpserver_->setErrorCallBack(std::bind(&EchoServer::errorCallBack, this, std::placeholders::_1));
    tcpserver_->setProcessCallBack(std::bind(&EchoServer::processCallBack, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_->setSendCompleteCallBack(std::bind(&EchoServer::sendCompleteCallBack, this, std::placeholders::_1));
}

EchoServer::~EchoServer()
{
}

void EchoServer::start()
{
    tcpserver_->start();
}
void EchoServer::acceptCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::closeCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::errorCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::processCallBack(SharedConnectionPointer conn, std::string message)
{
    // 这个判断是为了适应如果没有工作线程，那就必须由从线程自己来负责处理数据并发送数据的工作
    if (threadpool_->getThreadSize() == 0)
    {
        // 没有工作线程的情况
        logger.logMessage(DEBUG, __FILE__, __LINE__, "there is no worker thread, the sub thread will be responsible for handling and sending data itself.");
        wokerThreadBehavior(conn, message);
    }
    else
    {
        // 有工作线程的情况
        logger.logMessage(DEBUG, __FILE__, __LINE__, "EchoServer::processCallBack() called, sub thread id=%d", get_tid());
        threadpool_->addTask(std::bind(&EchoServer::wokerThreadBehavior, this, conn, message));
        logger.logMessage(DEBUG, __FILE__, __LINE__, "EchoServer threadpool addTask to task queue, sub thread id=%d", get_tid());
    }
}

void EchoServer::sendCompleteCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::epollTimeoutCallBack(EventLoop *loop)
{
}
void EchoServer::wokerThreadBehavior(SharedConnectionPointer conn, std::string message)
{
    logger.logMessage(DEBUG, __FILE__, __LINE__, "EchoServer::workerThreadBehavior() called, worker thread id=%d", get_tid());
    message = "reply:" + message;
    // 有可能是工作线程或者从线程执行下面这段代码
    conn->send(message);
}