#include "channel.hpp"

Channel::Channel(std::shared_ptr<EventLoop> loop, int fd) : loop_(loop), fd_(fd)
{
}
Channel::~Channel()
{
    // 注意在析构函数中不要销毁ep_，也不能关闭fd_，因为他们不属于Channel类，Channel类只是需要他们，使用他们而已
}

int Channel::fd()
{
    return fd_;
}

void Channel::setETMode()
{
    events_ = events_ | EPOLLET;
}

void Channel::registerReadEvent()
{
    events_ = events_ | EPOLLIN;
    loop_->updateChannel(this);
}

void Channel::unregisterReadEvent()
{
    events_ &= ~EPOLLIN;
    loop_->updateChannel(this);
}

void Channel::registerWriteEvent()
{
    events_ |= EPOLLOUT;
    loop_->updateChannel(this);
}

void Channel::unregisterWriteEvent()
{
    events_ &= ~EPOLLOUT;
    loop_->updateChannel(this);
}

void Channel::setInEpoll()
{
    inepoll_ = true;
}

void Channel::setRevents(uint32_t revs)
{
    revents_ = revs;
}

bool Channel::getInEpoll()
{
    return inepoll_;
}

uint32_t Channel::getEvents()
{
    return events_;
}

uint16_t Channel::getRevents()
{
    return revents_;
}
void Channel::handleEvent()
{

    if (revents_ & EPOLLRDHUP)
    {
        closecallback_();
    }
    else if (revents_ & (EPOLLIN | EPOLLPRI))
    {
        readcallback_();
    }
    else if (revents_ & EPOLLOUT)
    {
        writeCallBack_();
    }
    else
    {
        errorcallback_();
    }
}

void Channel::setReadCallBack(std::function<void()> func)
{
    readcallback_ = func;
}

void Channel::setCloseCallBack(std::function<void()> func)
{
    closecallback_ = func;
}

void Channel::setErrorCallBack(std::function<void()> func)
{
    errorcallback_ = func;
}

void Channel::setWriteCallBack(std::function<void()> func)
{
    writeCallBack_ = func;
}

void Channel::unregisterAll(){
    events_ = 0;
    loop_->updateChannel(this);
}

void Channel::removeSelfFromLoop(){
    loop_->removeChannel(this);
}