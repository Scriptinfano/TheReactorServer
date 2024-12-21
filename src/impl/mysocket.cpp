#include "mysocket.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <iostream>
#include <strings.h>
#include <fcntl.h>
#include "log.hpp"
using namespace std;
int createNonBlockingSocket()
{
    int listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (listenfd < 0)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, "socket() failed");
        exit(-1);
    }
    return listenfd;
}
Socket::Socket(int fd) : fd_(fd)
{
}

Socket::~Socket()
{
    close(fd_);
}

int Socket::getFd() const
{
    return fd_;
}

std::string Socket::getIP() const
{
    return ip_;
}

in_port_t Socket::getPort() const
{
    return port_;
}
void Socket::setNonBlocking(bool on)
{
    int flags;
    // 获取当前的文件描述符标志
    if ((flags = fcntl(fd_, F_GETFL, 0)) == -1)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("fcntl()获取文件描述符状态时出错").c_str());
        exit(-1);
    }

    if (on)
    {
        // 设置非阻塞模式
        if (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) == -1)
        {
            logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("fcntl()设置文件描述符阻塞状态时出错").c_str());
            exit(-1);
        }
    }
    else
    {
        // 清除非阻塞模式
        if (fcntl(fd_, F_SETFL, flags & ~O_NONBLOCK) == -1)
        {
            logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("fcntl()清除文件描述符非阻塞状态时出错").c_str());
            exit(-1);
        }
    }
}
void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}
void Socket::setReusePort(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}
void Socket::setTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}
void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}
void Socket::bind(const InetAddress &servaddr)
{
    if (::bind(fd_, servaddr.addr(), sizeof(sockaddr)) < 0)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("bind() failed").c_str());
        exit(-1);
    }
    ip_ = servaddr.ip();
    port_ = servaddr.port();
}
void Socket::listen(int backlog)
{
    if (::listen(fd_, backlog) < 0)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("listen() failed").c_str());
        exit(-1);
    }
}
int Socket::accept(InetAddress &clientaddr)
{
    struct sockaddr_in peeraddr;
    socklen_t len = sizeof(peeraddr);
    bzero(&peeraddr, sizeof(peeraddr));
    // accept4函数支持给新接受的套接字设置一个选项
    int clientfd = ::accept(fd_, (struct sockaddr *)&peeraddr, &len);
    if (clientfd < 0)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("accept failed").c_str());
        exit(-1);
    }
    clientaddr.setaddr(peeraddr);

    return clientfd;
}
void Socket::setIp(std::string ip){
    ip_ = ip;
}
void Socket::setPort(in_port_t port){
    port_ = port;
}