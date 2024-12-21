#pragma once
#include <sys/sem.h>
#include "myerror.hpp"

class Semaphore
{
private:
    /*
    union semun是一种常见的union类型，用于semctl函数中配置或或获取信号量的信息
    */
    union semun
    {
        int val;               // 信号量的值，用于SETVAL操作，设置信号量的单个整数值
        struct semid_ds *buf;  // 描述信号量的属性和状态，用于IPC_STAT或IPC_SET操作，分别用于读取或设置信号量状态信息
        unsigned short *array; // 指向一个数组，操作信号量集，用于GETALL和SETALL操作，获取或设置信号量的值
    };
    int semid;
    /*
    如果把sem_flg设置为SEM_UNDO，操作系统将跟踪进程对信号量的修改情况，在全部修改过信号量的进程终止后，操作系统将把信号量恢复为初始值。
    如果信号量用于互斥锁，设置为SEM_UNDO
    如果信号量用于生产消费者模型，设为0，如果设为0则所有进程对信号量修改完成并退出之后是不会被操作系统改为原值的
    */
    short semflg;
    Semaphore(const Semaphore &) = delete;
    Semaphore &operator=(const Semaphore &) = delete;

public:
    Semaphore() : semid(-1) {}
    /*
    信号量的初始化分为三个步骤：
    1. 获取信号量，如果成功则函数返回
    2. 如果失败则创建信号量
    3. 设置信号量的初始值
    本函数the_semflg的默认值是SEM_UNDO，说明这个信号量默认用于互斥锁
    @param key key_t类型的键值由ftok函数生成
    @param value 信号量的初始值
    @param the_semflg 控制标志，用于指定权限和行为；
    IPC_CREAT：若指定的 key 尚未存在，则创建一个新的信号量集；若已存在，则获取它；
    IPC_EXCL：与 IPC_CREAT 一起使用，若指定的 key 已存在则返回错误，否则创建一个新的信号量集;
    SEM_UNDO：操作系统跟踪进程对信号量的修改情况，在全部修改过信号量的进程终止后，操作系统将信号量恢复为初始值
    */
    bool init(key_t key, unsigned short value = 1, short the_semflg = SEM_UNDO);
    /*
    信号量的P操作，将信号量的值-value，如果信号量的值是0则阻塞等待，直到信号量的值大于0
    */
    bool wait(short value = -1);
    /*
    信号量的V操作，将信号量的值+value
    */
    bool post(short value = 1);
    /*
    获取信号量的值，成功则返回信号量的值，失败返回-1
    */
    int getvalue();
    /*
    销毁信号量
    */
    bool destroy();
};

void print_sem(Semaphore &sem, std::string name);