#pragma once
#include "eventloop.hpp"
#include "mysocket.hpp"
#include "channel.hpp"
#include <functional>
#include <memory>
class Accepter
{

private:
    std::shared_ptr<EventLoop> loop_;  // 主从Reactor多线程模型中负责监听传入连接的上层类Acceptor，由主进程负责运行事件循环
    std::unique_ptr<Socket> servsock_; // 负责监听传入连接的服务端套接字
    std::unique_ptr<Channel> acceptchannel_;//管理epoll的channel
    std::function<void(int, InetAddress &)> acceptCallBack_; // 回调到TCPServer中，在上层类中构建Connection接管连接
public:
    /*
    Accepter一旦初始化，内部的servsock就根据设置初始化完成并开始监听
    */
    Accepter(std::shared_ptr<EventLoop> loop, const std::string &ip, const in_port_t port);
    ~Accepter();
    /*
    实际调用servsock_的accept函数来接受连接，得到代表客户端的文件描述符
    */
    void handleNewConnection();
    void setAcceptCallBack(std::function<void(int, InetAddress &)> acceptCallBack);
};