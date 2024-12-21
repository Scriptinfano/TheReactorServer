#include "inetaddress.hpp"
#include "log.hpp"
#include <arpa/inet.h>
#include <iostream>
#include <cerrno>
#include <cstring>
InetAddress::InetAddress()
{
    memset(&addr_, 0, sizeof(addr_));
}


InetAddress::InetAddress(const std::string &ip, in_port_t port)
{
    addr_.sin_family = AF_INET;

    int ret = inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr);

    if (ret == 0)
    {
        throw std::invalid_argument("不是有效的地址格式，重新调用命令输入");
    }
    else if (ret == -1)
    {
        throw std::runtime_error("inet_pton() failed");
    }

    addr_.sin_port = htons(port);
}

InetAddress::InetAddress(const sockaddr_in addr) : addr_(addr)
{
}

std::string InetAddress::ip() const
{
    char str[INET_ADDRSTRLEN]; // 使用栈分配的数组
    const char *ip = inet_ntop(AF_INET, &(addr_.sin_addr), str, sizeof(str));

    if (ip == nullptr)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, "inet_ntop() failed");
        exit(-1);
    }
    return std::string(str); // 返回 std::string 类型
}

in_port_t InetAddress::port() const
{
    return ntohs(addr_.sin_port);
}
const sockaddr *InetAddress::addr() const
{
    return (sockaddr *)&addr_;
}
void InetAddress::setaddr(sockaddr_in clientaddr)
{
    addr_ = clientaddr;
}