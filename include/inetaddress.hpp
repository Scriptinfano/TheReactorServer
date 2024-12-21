#pragma once
#include <netinet/in.h>
#include <string>
/*
地址封装类
*/
class InetAddress
{
private:
    sockaddr_in addr_;

public:
    InetAddress();
    /*
    @brief 此构造函数用于构造监听套接字的地址结构
    */
    InetAddress(const std::string &ip, in_port_t port);
    /*
    @brief 此构造函数用于构造客户端连接套接字的地址结构
    */
    InetAddress(const sockaddr_in addr);
    /*
    @brief 返回地址表示的ip地址
    */
    std::string ip() const;
    /*
    返回地址中的port
    */
    in_port_t port() const;
    /*
    @brief 返回addr_成员的地址，转换成了sockaddr
    */
    const sockaddr *addr() const;
    /*
    设置addr_成员的值
    */
    void setaddr(sockaddr_in clientaddr);
};
