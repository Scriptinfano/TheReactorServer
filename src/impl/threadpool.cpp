#include "threadpool.hpp"
#include "log.hpp"
#include <unistd.h>
#include <sys/syscall.h>
#include <vector>
ThreadPool::ThreadPool(size_t threadnum,std::string threadtype)
{
    // 启动threadnum个线程，将每个线程阻塞在条件变量上
    for (size_t i = 0; i < threadnum; i++)
    {
        // emplace_back用于在容器末尾直接构造一个新元素，其会直接在内存中构造元素，避免了额外的临时对象创建和复制或移动操作
        // 下面传入的是一个lambda函数，[捕获列表](参数列表)->返回类型{函数体};捕获列表定义可以访问的外部变量
        threads_.emplace_back([this,threadtype]
                              {
                                // 另一种获取线程ID的方式：包含unistd.h和sys/syscall.h之后，使用syscall(SYS_gittid)获取
                                //推荐使用syscall(SYS_gettid)获取更合适，代码量更少，且两种方式获取的线程id的值不太一样，
                                logger.logMessage(DEBUG, __FILE__, __LINE__, "%s created, thread id=%d",threadtype.c_str(), syscall(SYS_gettid));
                                while(stop_==false){
                                    std::function<void()> task;
                                    {
                                        //锁作用域的开始
                                        std::unique_lock<std::mutex> lock(this->mutex_);//互斥锁加锁
                                        this->condition_.wait(lock, [this]
                                                              { return (this->stop_ == true) || this->taskqueue_.empty() == false; });
                                        //在上面线程是因为stop_被置为true苏醒，现在到这里又发现任务队列为空，则立即返回退出
                                        if(this->stop_==true && this->taskqueue_.empty()==true)
                                            return;
                                        //std::move函数主要用于将对象转换为右值引用，支持移动语义，这是为了避免拷贝带来的性能开销，使得task直接接管taskqueue_.front()
                                        task = std::move(this->taskqueue_.front());
                                        this->taskqueue_.pop();
                                    }
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