#pragma once
#include "tcpserver.hpp"
#include "threadpool.hpp"
// 这是一个定义业务类的头文件，想要修改整个服务器的业务处理逻辑需要在这里创建业务类，然后在相关回调函数中调用业务类即可
class EchoServer
{
private:
    
    std::unique_ptr<TCPServer> tcpserver_;
    std::unique_ptr<ThreadPool> threadpool_;//工作线程的线程池，负责处理客户端发来的数据

public:
    /*
    @param ip 要在哪一个ip上监听传入连接
    @param port 要在哪一个端口上监听传入连接
    @param threadnum 要开多少个子线程，子线程的epoll负责运行事件循环，监听客户端是否要发数据
    @param workthreadnum 工作线程的数量，收到客户端的数据后，工作线程负责处理客户端的数据
    */
    EchoServer(const std::string &ip, in_port_t port,int threadnum,int workthreadnum);
    ~EchoServer();
    /*
    在Accepter初步accept之后，接下来的处理工作
    */
    void start();

    void acceptCallBack(SharedConnectionPointer conn);

    void closeCallBack(SharedConnectionPointer conn);

    void errorCallBack(SharedConnectionPointer conn);
    /*
    该函数将数据处理的工作进一步交给工作线程处理，工作线程需要一个执行函数，即本类下的wokerThreadBehavior()
    @param conn 处理哪一个连接发来的数据
    @param message 原始数据
    */
    void processCallBack(SharedConnectionPointer conn, std::string message);

    /*
    当Connection将数据都加到输入缓冲区中之后，回调这个函数，相当于通知TCPServer
    */
    void sendCompleteCallBack(SharedConnectionPointer conn);

    /*
    在EventLoop中如果发生超时的情况，需要回调这个函数
    */
    void epollTimeoutCallBack(EventLoop *loop);

    /*
    工作线程的主代码，如果没有工作线程，则这个函数的代码将由从线程执行
    */
    void wokerThreadBehavior(SharedConnectionPointer conn, std::string message);
};
