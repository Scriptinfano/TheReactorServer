#pragma once
#include <sys/epoll.h>
#include <vector>
#include "channel.hpp"
class Channel; // 如果两个头文件互相包含，互相需要对方的数据结构，那么需要在两个文件做对方的前向声明，而且要在头文件的首部加入#pragme once
/*
对epoll相关底层API进行封装
*/
class Epoll
{
private:
    static const int MAXEVENTS = 100;//定义每一个epoll可以监听的最大文件描述符的数量
    int epollfd_ = -1;//epfd，调用epoll_create之后返回的文件描述符
    epoll_event events_[MAXEVENTS] = {};//epoll_wait的第二个参数，用来接收由内核返回的事件数据，需要预先分配足够的内存空间

public:
    Epoll();
    ~Epoll();
    /*
    根据Channel的内部成员所包含的信息，内部自行构造epoll_event然后将其添加或更新到红黑树上
    */
    void updateChannel(Channel *ch);
    /*
    在函数内部调用epoll_wait，将events中包含的信息传回Channel对象，并将Channel集合返回
    @param timeout 内部调用epoll_wait的时候的超时参数，单位是毫秒
    */
    std::vector<Channel *> loop(int timeout = -1);

    /*
    将监控节点从底层红黑树中删除
    */
    void removeChannel(Channel *ch);
};