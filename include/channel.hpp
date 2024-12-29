#pragma once
#include "eventloop.hpp"
#include <functional>
#include <memory>
class EventLoop;
/*
对连接抽象化的Channel类，Channel类是Accepter和Connection的下层类，代表要对文件描述符监视哪种事件，事件循环和Channel是一对多的关系
*/
class Channel
{
private:
    int fd_ = -1;                         // Channel和fd是一对一的关系
    bool inepoll_ = false;                // 标记Channel是否已添加到epoll的红黑树上，如果没有添加，则调用epoll_ct的时候添加，否则用EPOLL_CTL_MOD
    uint32_t events_ = 0;                 // 需要监视的事件类型，listenfd和clientfd需要监视EPOLLIN，clientfd还需要监视EPOLLOUT，当写就绪事件发生时，便将写缓冲区的数据一次性发送出去
    uint32_t revents_ = 0;                // 已经就绪的事件
    std::shared_ptr<EventLoop> loop_;     // channel需要通知事件循环对象根据自己承载的信息更新epoll树
    std::function<void()> readcallback_;  // 遇到可读事件的回调函数
    std::function<void()> closecallback_; // 客户端连接关闭时的回调处理函数：Connection::closeCallBack()
    std::function<void()> errorcallback_; // 在处理处理客户端连接可读事件过程中发生其他错误的回调处理函数：Connection::errorCallBack()
    std::function<void()> writeCallBack_; // 可写事件发生时的回调函数
public:
    /*
    @param loop 该Channel属于哪一个事件循环
    @param fd 该Channel需要关联的文件描述符是哪一个
    @param sock 该Channel需要关联的文件描述符对应的封装套接字是哪一个
    */
    Channel(std::shared_ptr<EventLoop> loop, int fd);
    ~Channel();

    /*
    @brief 取得fd成员
    */
    int fd();
    /*
    @brief 设置采用边缘触发模式
    */
    void setETMode();
    /*
    @brief 函数内部会调用已绑定epoll实例，将读就绪事件添加到epoll的红黑树中，如果已经有了，则会修改红黑树
    */
    void registerReadEvent();

    void unregisterReadEvent();

    void registerWriteEvent();

    void unregisterWriteEvent();

    /*
    彻底取消监视各个类型的事件
    */
    void unregisterAll();

    /*
    从事件循环中删除此Channel
    */
    void removeSelfFromLoop();

    /*
    @brief 将inepoll的值设为true
    */
    void setInEpoll();
    /*
    @brief 设置内部revents，承载epoll_wait返回的事件
    */
    void setRevents(uint32_t revs);
    /*
    @brief 将inepoll的值设为true，表示添加到了树上
    */
    bool getInEpoll();
    /*
    @brief 返回events成员
    */
    uint32_t getEvents();
    /*
    @brief 返回revents成员
    */
    uint16_t getRevents();
    /*
    @brief 事件处理函数，epoll_wait()返回的时候执行它
    */
    void handleEvent();

    /*
    设置读事件的回调函数
    */
    void setReadCallBack(std::function<void()> func);

    void setCloseCallBack(std::function<void()> func);

    void setErrorCallBack(std::function<void()> func);

    void setWriteCallBack(std::function<void()> func);
};