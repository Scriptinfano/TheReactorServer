#pragma once
#include "eventloop.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
#include "buffer.hpp"
#include <memory>
#include <atomic>
class Connection;
using SharedConnectionPointer = std::shared_ptr<Connection>;
/*
代表和客户端之间的抽象连接，其成员事件循环负责监听该客户的各种类型的事件，一种事件就代表一种Channel，负责管理自定义发送缓冲区和自定义接收缓冲区
继承std::enable_shared_from_this<T>的目的是为了让该类的对象能安全通过shared_from_this()方法获取指向自身的智能指针
*/
class Connection:public std::enable_shared_from_this<Connection>
{
private:
    std::shared_ptr<EventLoop> loop_;//此事件循环负责监听已连接的客户端的读事件
    std::unique_ptr<Socket> clientsock_;     // 客户端连接的套接字，通过make_unique调用Socket的构造函数将地址赋给智能指针
    std::unique_ptr<Channel> clientchannel_; // 客户级别的Channel
    std::function<void(SharedConnectionPointer)> closeCallBack_;//回调到TCPServer的同名函数中
    std::function<void(SharedConnectionPointer)> errorCallBack_;//回调到TCPServer的同名函数中
    std::function<void(SharedConnectionPointer, std::string&)> processCallBack_; // 处理客户端发来的数据的回调函数，回调到TCPServer中
    std::function<void(SharedConnectionPointer)> sendCompleteCallBack_;
    Buffer inputBuffer_; // 接收缓冲区，每次调用read需要读多次，读一次没有读完就先放到读缓冲区中
    Buffer outputBuffer_; // 发送缓冲区，每次工作线程处理完业务数据之后将处理之后的业务数据会先写在写缓冲区中，只有下一次写事件就绪之后才会一次性地被从线程发送出去
    std::atomic_bool disconnect_;//标记客户端连接是否已断开，如果已断开则设置为true

public:
    /*
    @param loop 这个Connection属于哪一个事件循环
    @param fd 客户端连接的文件描述符
    */
    Connection(std::shared_ptr<EventLoop> loop, int fd, InetAddress *clientaddr);
    ~Connection();
    int getFd() const;
    std::string getIP() const;
    in_port_t getPort() const;
    /*
    TCP连接关闭断开之后的回调函数，供Channel回调
    */
    void closeCallBack();
    /*
    TCP连接错误的回调函数，供Channel回调
    */
    void errorCallBack();
    /*
    设定当连接关闭的时候该执行的回调函数
    */
    void setCloseCallBack(std::function<void(SharedConnectionPointer)> closeCallBack);
    /*
    设定当错误发生时该执行的回调函数
    */
    void setErrorCallBack(std::function<void(SharedConnectionPointer)> errorCallBack);
    /*
    设定该如何处理客户端的数据
    */
    void setProcessCallBack(std::function<void(SharedConnectionPointer, std::string&)> processCallBack);

    void setSendCompleteCallBack(std::function<void(SharedConnectionPointer)> sendCompleteCallBack);
    /*
    此函数是真正调用read函数的
    */
    void readCallBack();

    /*
    将outputBuffer_中积攒的数据实际发送出去
    */
    void writeCallBack();
    /*
    不管线程是从线程还是工作线程，发送数据的第一步一定是先调用这个函数，在里面判断是哪一种线程在调用这个函数，决定是直接将数据放入自定义输出缓冲区还是调用异步事件通知机制让从线程发送数据（工作线程不能直接操作自定义输出缓冲区，可能造成线程不安全）
    */
    void send(std::string data);

    /*
    将报头和处理之后的数据合在一起放到自定义输出缓冲区中，然后注册clientchannel_的可写监听事件
    */
    void sendInIOThread(std::string data);
};