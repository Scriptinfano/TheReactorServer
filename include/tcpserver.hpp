#pragma once
#include "eventloop.hpp"
#include "accepter.hpp"
#include "connection.hpp"
#include <map>
#include <vector>
#include "threadpool.hpp"
/*
顶层封装，整个业务服务器的接口提供类，如果要增加其他业务，需要在业务类中包含该类对象，然后调用set...CallBack()函数设置回调函数到自己的类成员函数
*/
class TCPServer
{
private:
    std::shared_ptr<EventLoop> loop_;                         // 在主从多线程Reactor模型下，该事件循环负责监听是否新连接传入
    std::vector<std::shared_ptr<EventLoop>> subloops_;        // 从事件循环集合，每个从事件循环都在监听已连接的客户端的可读或者其他事件是否就绪，如果就绪则接收数据，然后将处理业务数据的任务提交给工作线程池的任务队列，让其中的工作线程处理数据的处理
    std::unique_ptr<Accepter> accepter_;                      // 一个TCPServer只有一个Accepter类
    std::unique_ptr<ThreadPool> threadpool_;                  // 从线程的线程池
    std::map<int, SharedConnectionPointer> connectionMapper_; // 一个TCPServer可以有多个Connection，所以用一个Map容器将Connection管理起来
    int threadnum_;                                           // 从线程的线程池中应该有多少个线程
    std::function<void(SharedConnectionPointer)> acceptCallBack_;
    std::function<void(SharedConnectionPointer)> closeCallBack_;
    std::function<void(SharedConnectionPointer)> errorCallBack_;
    std::function<void(SharedConnectionPointer, std::string &, bool)> processCallBack_;
    std::function<void(SharedConnectionPointer)> sendCompleteCallBack_;
    std::function<void(EventLoop *)> epollTimeoutCallBack_;

public:
    TCPServer(const std::string &ip, const in_port_t port, int threadnum);
    ~TCPServer();
    /*
    开始运行事件循环，也就是开始运行这个服务器
    */
    void start();
    /*
    Accepter让servsock_调用accept函数之后会回调到这个函数上，这个函数将创建Connection对象，将从事件循环分配给Connection对象
    */
    void acceptCallBack(int fd, InetAddress clientaddr);
    /*

    */
    void closeCallBack(SharedConnectionPointer scp);
    /*

    */
    void errorCallBack(SharedConnectionPointer scp);
    /*
    该函数代表整个服务器对于客户端发来的数据的一个处理
    @param conn 处理哪一个连接发来的数据
    @param message 原始数据
    */
    void processCallBack(SharedConnectionPointer scp, std::string &message,bool hasHead);

    /*
    当Connection将数据都加到输入缓冲区中之后，回调这个函数，相当于通知TCPServer
    */
    void sendCompleteCallBack(SharedConnectionPointer scp);

    /*
    在EventLoop中如果发生超时的情况，需要回调这个函数
    */
    void epollTimeoutCallBack(EventLoop *loop);

    void setAcceptCallBack(std::function<void(SharedConnectionPointer)> acceptCallBack);
    void setCloseCallBack(std::function<void(SharedConnectionPointer)> closeCallBack);
    void setErrorCallBack(std::function<void(SharedConnectionPointer)> errorCallBack);
    void setProcessCallBack(std::function<void(SharedConnectionPointer, std::string &,bool)> processCallBack);
    void setSendCompleteCallBack(std::function<void(SharedConnectionPointer)> sendCompleteCallBack);
    void setEpollTimeoutCallBack(std::function<void(EventLoop *)> epollTimeoutCallBack);
};