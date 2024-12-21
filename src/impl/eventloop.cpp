#include "eventloop.hpp"
#include "log.hpp"
#include <sys/syscall.h>
#include <unistd.h>
#include <mutex>
#include <sys/eventfd.h>
#include <memory>
EventLoop::EventLoop() : ep_(std::make_unique<Epoll>()), wakeupfd_(eventfd(0, EFD_NONBLOCK))
{
}

EventLoop::~EventLoop()
{
}

void EventLoop::run()
{
    threadid_ = syscall(SYS_gettid);
    logger.logMessage(DEBUG, __FILE__, __LINE__, "EventLoop::run() called, thread is %d", syscall(SYS_gettid));
    while (true)
    {
        std::vector<Channel *> chans = ep_->loop(10 * 1000); // 开始等待就绪事件发生
        // 如果chans是空的，那么应该回调TCPServer中的epolltimeout函数
        if (chans.empty())
        {
            epollTimeoutCallBack_(this);
        }
        // 开始遍历epoll_event数组获得就绪的文件描述符信息
        for (auto &ch : chans)
        {
            ch->handleEvent();
        }
    }
}
void EventLoop::updateChannel(Channel *chan)
{
    ep_->updateChannel(chan);
}
void EventLoop::setEpollTimeoutCallBack(std::function<void(EventLoop *)> epollTimeoutCallBack)
{
    epollTimeoutCallBack_ = epollTimeoutCallBack;
}
void EventLoop::removeChannel(Channel *chan)
{
    ep_->removeChannel(chan);
}
bool EventLoop::isIOThread()
{
    return threadid_ == syscall(SYS_gettid);
}

void EventLoop::addTaskToQueue(std::function<void(std::string)> fn,std::string data)
{
    {
        std::lock_guard<std::mutex> guard(mutex_);
        std::pair<std::function<void(std::string)>, std::string> mypair(fn, data);
        taskQueue_.push(mypair);
    }
    // 唤醒从线程
    wakeup();
}

void EventLoop::wakeup()
{
    uint64_t val = 1; // eventfd计数器的值必须是一个uint64_t类型的整型
    write(wakeupfd_, &val, sizeof(val));
}

void EventLoop::handleWakeUp()
{
    logger.logMessage(DEBUG, __FILE__, __LINE__, "EventLoop::handleWakeUp() called, thread id is %d.", syscall(SYS_gettid));
    uint64_t val;
    read(wakeupfd_, &val, sizeof(val)); // 读出wakeupfd_的值，如果不读取，那么这个值不会清零，相当于唤醒的闹铃声一直不关
    
    std::lock_guard<std::mutex> guard(mutex_); // 给任务队列加锁
    while (taskQueue_.size() > 0)
    {
        auto pair = std::move(taskQueue_.front()); // 如果函数对象在调用std::bind的时候捕获了复杂的变量，那内部就需要存储，std::move避免了拷贝操作，使得fn直接接管了内部的资源
        taskQueue_.pop();
        auto fn=pair.first;
        auto str = pair.second;
        logger.logMessage(DEBUG, __FILE__, __LINE__, "已经将任务从任务队列中取出，数据部分为%s",str.c_str());
        fn(str);
    }
}
void EventLoop::setWakeChannel()
{
    wakeChannel_ = std::make_unique<Channel>(shared_from_this(), wakeupfd_);
    wakeChannel_->setReadCallBack(std::bind(&EventLoop::handleWakeUp, this));
    wakeChannel_->registerReadEvent();
}