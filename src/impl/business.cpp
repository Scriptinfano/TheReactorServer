#include "business.hpp"
#include "log.hpp"
#include <sys/syscall.h>
#include <unistd.h>
#include <iostream>
BusinessServerInterface::BusinessServerInterface(const std::string &ip, in_port_t port, int threadnum, int workthreadnum)
BusinessServerInterface::BusinessServerInterface(const std::string &ip, in_port_t port, int threadnum, int workthreadnum)
{
    tcpserver_ = std::make_unique<TCPServer>(ip, port, threadnum);
    threadpool_ = std::make_unique<ThreadPool>(workthreadnum, "worker_thread");
    // 让TCPServer最后再回调到自己的函数中
    tcpserver_->setAcceptCallBack(std::bind(&BusinessServerInterface::acceptCallBack, this, std::placeholders::_1));
    tcpserver_->setCloseCallBack(std::bind(&BusinessServerInterface::closeCallBack, this, std::placeholders::_1));
    tcpserver_->setEpollTimeoutCallBack(std::bind(&BusinessServerInterface::epollTimeoutCallBack, this, std::placeholders::_1));
    tcpserver_->setErrorCallBack(std::bind(&BusinessServerInterface::errorCallBack, this, std::placeholders::_1));
    tcpserver_->setProcessCallBack(std::bind(&BusinessServerInterface::processCallBack, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_->setSendCompleteCallBack(std::bind(&BusinessServerInterface::sendCompleteCallBack, this, std::placeholders::_1));
    tcpserver_->setAcceptCallBack(std::bind(&BusinessServerInterface::acceptCallBack, this, std::placeholders::_1));
    tcpserver_->setCloseCallBack(std::bind(&BusinessServerInterface::closeCallBack, this, std::placeholders::_1));
    tcpserver_->setEpollTimeoutCallBack(std::bind(&BusinessServerInterface::epollTimeoutCallBack, this, std::placeholders::_1));
    tcpserver_->setErrorCallBack(std::bind(&BusinessServerInterface::errorCallBack, this, std::placeholders::_1));
    tcpserver_->setProcessCallBack(std::bind(&BusinessServerInterface::processCallBack, this, std::placeholders::_1, std::placeholders::_2));
    tcpserver_->setSendCompleteCallBack(std::bind(&BusinessServerInterface::sendCompleteCallBack, this, std::placeholders::_1));
}
void BusinessServerInterface::processCallBack(SharedConnectionPointer conn, std::string message)
void BusinessServerInterface::processCallBack(SharedConnectionPointer conn, std::string message)
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
        logger.logMessage(DEBUG, __FILE__, __LINE__, "EchoServer::processCallBack() called, sub thread id=%d", syscall(SYS_gettid));
        threadpool_->addTask([this, conn, message]()
                             { wokerThreadBehavior(conn, message); });
        logger.logMessage(DEBUG, __FILE__, __LINE__, "EchoServer threadpool addTask to task queue, sub thread id=%d", syscall(SYS_gettid));
    }
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

void EchoServer::acceptCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::closeCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::errorCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::sendCompleteCallBack(SharedConnectionPointer conn)
{
}

void EchoServer::epollTimeoutCallBack(EventLoop *loop)
{
}
void EchoServer::wokerThreadBehavior(SharedConnectionPointer conn, std::string message)
{
    logger.logMessage(DEBUG, __FILE__, __LINE__, "EchoServer::workerThreadBehavior() called, worker thread id=%d", syscall(SYS_gettid));
    message = "reply:" + message;
    // 有可能是工作线程或者从线程执行下面这段代码
    conn->send(message);
}
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void HttpServer::acceptCallBack(SharedConnectionPointer conn)
{
}

void HttpServer::closeCallBack(SharedConnectionPointer conn)
{
}

void HttpServer::errorCallBack(SharedConnectionPointer conn)
{
}

void HttpServer::sendCompleteCallBack(SharedConnectionPointer conn)
{
}

void HttpServer::epollTimeoutCallBack(EventLoop *loop)
{
}

void HttpServer::wokerThreadBehavior(SharedConnectionPointer conn, std::string message)
{
    // 处理http请求
}