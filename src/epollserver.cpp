
#include <iostream>
#include "business.hpp"
#include "log.hpp"
using namespace std;

int main(int argc, char **argv)
{
    // 检查参数的合法性
    if (argc != 3)
    {
        cout << "usage: epollserver ip port" << endl;
        cout << "example, epollserver 192.168.150.128 8080" << endl;
        return -1;
    }
    //////////////
    Logger::setLoggerPname(argv[0]);
    EchoServer server(argv[1], atoi(argv[2]), 5, 5);
    server.start();
}