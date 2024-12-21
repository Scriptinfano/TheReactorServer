#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
class ThreadPool
{
private:
    /*
    线程池中的线程
    */
    std::vector<std::thread> threads_;
    /*
    queue是一种先进先出的队列容器
    */
    std::queue<std::function<void()>> taskqueue_;
    /*
    任务队列的互斥锁
    */
    std::mutex mutex_;
    /*
    任务队列的条件变量
    */
    std::condition_variable condition_;
    /*
    在析构函数中将stop_的值设为true，如果想让线程池停止工作，将其置为true
    */
    std::atomic_bool stop_;

    /*
    区分线程是子线程还是工作线程，两种取值，“io_thread”,"worker_thread"
    */
    std::string threadtype_;

public:
    /*
    在构造函数中启动threadnum个线程
    */
    ThreadPool(size_t threadnum,std::string threadtype);
    /*
    在析构函数中终止线程
    */
    ~ThreadPool();
    /*
    将任务添加到队列中
    */
    void addTask(std::function<void()> task);
    /*
    得到此时线程池中线程的数量
    */
    int getThreadSize();
};
