
#include <iostream>
#include "business.hpp"
#include "log.hpp"
using namespace std;
static const int ACQUIREARGNUM = 3;
int main(int argc, char **argv)
{
    // 检查参数的合法性
    if (argc != ACQUIREARGNUM)
    {
        cout << "argc invalid:" << argc << endl;
        cout << "usage: epollserver ip port" << endl;
        cout << "example, epollserver 192.168.150.128 8080" << endl;
        return -1;
    }
    //////////////
    Logger::setLoggerPname(argv[0]);
    BusinessServerInterface *server = new EchoServer(argv[1], atoi(argv[2]), 4, 4);
    // BusinessServerInterface *server2 = new HttpServer(argv[1], atoi(argv[3]), 4, 4);
    server->start();
    // server2->start();
}