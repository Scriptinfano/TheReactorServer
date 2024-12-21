#include <iostream>
#include <string>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "myio.hpp"
using namespace std;

static const int BUFFERSIZE = 1024;
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Usage: ./client <IP> <port>" << endl;
        cout << "Example: ./client 192.168.150.128 5085" << endl;
        return -1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    string buf;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cerr << "socket() failed: " << strerror(errno) << endl;
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
    {
        cerr << "Invalid IP address: " << argv[1] << endl;
        close(sockfd);
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        cerr << "connect(" << argv[1] << ":" << argv[2] << ") failed: " << strerror(errno) << endl;
        close(sockfd);
        return -1;
    }

    cout << "Connect successful." << endl;

    for (int i = 0; i < 1; i++)
    {
        char buffer[BUFFERSIZE] = {0};
        sprintf(buffer, "this is data which number is %d", i + 1);
        
        char tmp[BUFFERSIZE] = {0};
        int len = strlen(buffer);
        memcpy(tmp, &len, sizeof(int));
        memcpy(tmp + sizeof(int), buffer, strlen(buffer));
        mysend(sockfd, tmp, sizeof(int) + len);
        cout << "data has been sent, the strlen of data is" << strlen(buffer) << endl;
    }

    cout << "即将开始读取服务端的回复消息" << endl;
    while (true)
    {
        //先把报头读取出来
        int len;
        myrecv(sockfd, &len, sizeof(int));
        cout << "len=" << len<<"--";
        char recv_buf[len + 1] = {0}; // 这里一定要给recv_buf多留一个空字符的位置
        myrecv(sockfd, recv_buf, len);
        cout << "received reply msg from server:" << recv_buf << endl;
    }
    sleep(100);//睡过100s再断开连接

    close(sockfd);
    return 0;
}
