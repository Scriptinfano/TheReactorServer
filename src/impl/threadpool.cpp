#include "threadpool.hpp"
#include "log.hpp"
#include <unistd.h>
#include <vector>
#include "public.hpp"
#ifdef __linux__
#include <sys/syscall.h>
#endif
ThreadPool::ThreadPool(size_t threadnum, std::string threadtype) : threadtype_(threadtype), stop_(false)
{
    // 启动threadnum个线程，每个线程都阻塞在条件变量上
    for (size_t i = 0; i < threadnum; i++)
    {
        // 用lambda表达式创建线程
        threads_.emplace_back([this, threadtype]
                              {
                                logger.logMessage(DEBUG, __FILE__, __LINE__, "%s created, thread id=%d",threadtype.c_str(), get_tid());
                                while(stop_==false){
                                    std::function<void()> task; 
                                    {
                                        //在这个作用域内加锁，出了作用域自动解锁
                                        std::unique_lock<std::mutex> lock(this->mutex_);
                                        //等待生产者的条件变量
                                        this->condition_.wait(lock, [this]
                                                            { return this->stop_ || !this->taskqueue_.empty(); });
                                        if(this->stop_ && this->taskqueue_.empty()) return;
                                        task = std::move(this->taskqueue_.front());
                                        this->taskqueue_.pop();
                                    }
                                    logger.logMessage(DEBUG, __FILE__, __LINE__, "%s(%d) execute task",threadtype.c_str(), get_tid());
                                    task();
                                } });
    }
}

ThreadPool::~ThreadPool()
{
    stop_ = true;
    condition_.notify_all();
    // 给所有线程调用join()函数，相当于要等待所有线程执行完成之后才结束析构函数
    for (std::thread &th : threads_)
        th.join();
}

void ThreadPool::addTask(std::function<void()> task)
{
    {
        // lock_guard是一种线程锁管理工具，在作用域内自动管理互斥锁
        std::lock_guard<std::mutex> lock(mutex_);
        taskqueue_.push(task);
    }
    condition_.notify_one();
}

int ThreadPool::getThreadSize(){
    return threads_.size();
}