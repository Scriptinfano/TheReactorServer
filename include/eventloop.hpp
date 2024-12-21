#pragma once
#include "epoll.hpp"
#include "channel.hpp"
#include <functional>
#include <memory>
#include <queue>
#include <mutex>
class Channel;
class Epoll;
/*
事件循环
*/
class EventLoop:public std::enable_shared_from_this<EventLoop>
{
private:
    std::unique_ptr<Epoll> ep_; // 实际负责管理epoll。调用epoll相关底层接口的的底层对象
    std::function<void(EventLoop *)> epollTimeoutCallBack_;
    pid_t threadid_;                              // 标识这个事件循环属于哪一个线程，在run函数中初始化
    std::queue<std::pair<std::function<void(std::string)>,std::string>> taskQueue_; // 这个成员变量是专门给工作线程用的，因为要解决多线程互斥问题，所以工作线程在将处理之后的数据加到自定义输出缓冲区之后要靠eventfd唤醒从线程来发送数据
    std::mutex mutex_;                            // 用于对taskQueue_加锁的互斥锁
    int wakeupfd_;                                // 用于唤醒事件循环的eventfd
    std::unique_ptr<Channel> wakeChannel_;        // 这个Channel用来监听wakeupfd的可读事件，可读事件触发代表被工作线程通知可以发送输出缓冲区中的数据了 

public:
    EventLoop();

    ~EventLoop();
    /*
    运行事件循环
    */
    void run();

    /*
    根据Channel提供的信息将相应的事件添加到底层的epoll中
    */
    void updateChannel(Channel *chan);

    void setEpollTimeoutCallBack(std::function<void(EventLoop *)> epollTimeoutCallBack);

    /*
    将和chan相关节点从epoll中删除
    */
    void removeChannel(Channel *chan);
    /*
    判断当前事件循环是否在IO线程中
    */
    bool isIOThread();

    /*
    将任务添加到内部的任务队列中，从线程在唤醒之后会从队列中取出任务执行
    */
    void addTaskToQueue(std::function<void(std::string)> fn, std::string data);
    /*
    通过内部的wakeupfd_来唤醒另一处通过read阻塞着的代码
    */
    void wakeup();

    void handleWakeUp();

    void setWakeChannel();
};