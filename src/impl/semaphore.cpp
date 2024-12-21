#include "semaphore.hpp"
#include <cerrno>
#include <iostream>
bool Semaphore::init(key_t key, unsigned short value, short the_semflg)
{
    if (semid != -1)
        return false;
    semflg = the_semflg;
    /*
    semget各个参数解释
    key用来唯一标识信号量集
    1: 请求的信号量数量
    0666: 权限位
    IPC_CREAT: 如果信号量不存在则创建一个新的信号量集
    IPC_EXCL: 如果信号量集已经存在，则调用失败
    IPC_CREAT|IPC_EXCL 的使用会使得创建信号量时。如果信号量存在，则errno被设为EEXIST，然后返回-1
    */
    semid = semget(key, 1, 0666 | IPC_CREAT | IPC_EXCL);
    if (semid == -1)
    {
        if (errno == EEXIST)
        {
            // 信号量已存在的情况下再次尝试获取信号量
            semid = semget(key, 1, 0666);
            if (semid == -1)
            {
                // 再次尝试获取信号量之后失败
                print_error("init函数中调用semget获取已有信号量时失败");
                return false;
            }
            // 再次尝试获取信号量之后成功
        }
        else
        {
            print_error("init函数中发生了其他错误");
            return false;
        }
    }

    union semun sem_union;
    sem_union.val = value;
    if (semctl(semid, 0, SETVAL, sem_union) < 0)
        print_error("init函数中调用semctl()时失败");
    return true;
};
bool Semaphore::wait(short value)
{
    if (semid == -1)
        return false;

    /*
    sembuf是用于描述信号量操作的结构体，配合semop函数用于执行信号量的增减操作或等待操作
    */
    struct sembuf sem_b;
    sem_b.sem_num = 0;    // 信号量编号，0代表信号量集中的第一个信号量
    sem_b.sem_op = value; // 操作数，用于指定信号量的增减值
    sem_b.sem_flg = semflg;
    if (semop(semid, &sem_b, 1) == -1)
    {
        print_error("在wait函数中调用semop函数时出粗");
        return false;
    }
    return true;
}

bool Semaphore::post(short value)
{
    if (semid == -1)
        return false;
    struct sembuf sem_b;
    sem_b.sem_num = 0;
    sem_b.sem_op = value;
    sem_b.sem_flg = semflg;
    if (semop(semid, &sem_b, 1) == -1)
    {
        print_error("在函数post中调用semop函数时出错");
        return false;
    }
    return true;
}
int Semaphore::getvalue()
{
    return semctl(semid, 0, GETVAL);
}
bool Semaphore::destroy()
{
    if (semid == -1)
        return false;
    if (semctl(semid, 0, IPC_RMID) == -1)
    {
        print_error("在destroy函数中调用semctl销毁信号量失败");
        return false;
    }
    return true;
}

void print_sem(Semaphore &sem, std::string name)
{
    std::cout << "信号量" << name << "=" << sem.getvalue() << std::endl;
}