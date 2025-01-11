#pragma once
#include "tcpserver.hpp"
#include "threadpool.hpp"

class BusinessServerInterface
{
public:

    BusinessServerInterface(const std::string &ip, in_port_t port, int threadnum, int workthreadnum);
    virtual ~BusinessServerInterface() {}
    void processCallBack(SharedConnectionPointer conn, std::string message);
    void start()
    {
        tcpserver_->start();
    }

    virtual void acceptCallBack(SharedConnectionPointer conn) = 0;
    virtual void closeCallBack(SharedConnectionPointer conn) = 0;
    virtual void errorCallBack(SharedConnectionPointer conn) = 0;
    virtual void sendCompleteCallBack(SharedConnectionPointer conn) = 0;
    virtual void epollTimeoutCallBack(EventLoop *loop) = 0;
    virtual void wokerThreadBehavior(SharedConnectionPointer conn, std::string message) = 0;

protected:
    std::unique_ptr<TCPServer> tcpserver_; // 服务器的主类，负责监听传入连接
    std::unique_ptr<ThreadPool> threadpool_; // 工作线程的线程池，负责处理客户端发来的数据
};
// 这是一个定义业务类的头文件，想要修改整个服务器的业务处理逻辑需要在这里创建业务类，然后在相关回调函数中调用业务类即可
class EchoServer: public BusinessServerInterface
{

public:
    /*
    @param ip 要在哪一个ip上监听传入连接
    @param port 要在哪一个端口上监听传入连接
    @param threadnum 要开多少个子线程，子线程的epoll负责运行事件循环，监听客户端是否要发数据
    @param workthreadnum 工作线程的数量，收到客户端的数据后，工作线程负责处理客户端的数据
    */
    EchoServer(const std::string &ip, in_port_t port, int threadnum, int workthreadnum)
        : BusinessServerInterface(ip, port, threadnum, workthreadnum) {}
    void acceptCallBack(SharedConnectionPointer conn) override;
    void closeCallBack(SharedConnectionPointer conn) override;
    void errorCallBack(SharedConnectionPointer conn) override;
    void sendCompleteCallBack(SharedConnectionPointer conn) override;
    void epollTimeoutCallBack(EventLoop *loop) override;
    void wokerThreadBehavior(SharedConnectionPointer conn, std::string message) override;
};

class HttpServer: public BusinessServerInterface
{

public:
    /*
    @param ip 要在哪一个ip上监听传入连接
    @param port 要在哪一个端口上监听传入连接
    @param threadnum 要开多少个子线程，子线程的epoll负责运行事件循环，监听客户端是否要发数据
    @param workthreadnum 工作线程的数量，收到客户端的数据后，工作线程负责处理客户端的数据
    */
    HttpServer(const std::string &ip, in_port_t port, int threadnum, int workthreadnum)
        : BusinessServerInterface(ip, port, threadnum, workthreadnum) {}
    void acceptCallBack(SharedConnectionPointer conn) override;
    void closeCallBack(SharedConnectionPointer conn) override;
    void errorCallBack(SharedConnectionPointer conn) override;
    void sendCompleteCallBack(SharedConnectionPointer conn) override;
    void epollTimeoutCallBack(EventLoop * loop) override;
    void wokerThreadBehavior(SharedConnectionPointer conn, std::string message) override;
};