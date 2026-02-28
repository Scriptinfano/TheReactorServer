#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <thread>
#include "myio.hpp"
#include "public.hpp"
#include "chacha20.hpp"
using namespace std;

static const int BUFFERSIZE = 1024;
static const string KEY = "TheReactorServerSecretKey1234567"; // 32 bytes

void sendThread(int sockfd)
{
    string line;
    while (getline(cin, line))
    {
        if (line == "exit" || line == "quit")
        {
            break;
        }

        // 1. 加密消息
        string encrypted = ChaCha20::encrypt(line, KEY);

        // 2. 构造报文
        int len = encrypted.length();
        char tmp[BUFFERSIZE] = {0};
        
        // 防止溢出，虽然这里 BUFFERSIZE 是 1024，但还是稍微注意一下
        if (len + sizeof(int) > BUFFERSIZE) {
            cerr << "Message too long!" << endl;
            continue;
        }

        memcpy(tmp, &len, sizeof(int));
        memcpy(tmp + sizeof(int), encrypted.data(), len);

        // 3. 发送
        mysend(sockfd, tmp, sizeof(int) + len);
    }
}

void recvThread(int sockfd)
{
    while (true)
    {
        //先把报头读取出来
        int len;
        ssize_t ret = recv(sockfd, &len, sizeof(int), 0);
        if (ret <= 0)
        {
            cout << "Disconnected from server." << endl;
            exit(0);
        }

        if (len <= 0 || len > 65535) { // 简单的长度检查
             cerr << "Invalid message length: " << len << endl;
             continue; // 或者退出
        }

        std::vector<char> recv_buf(len, 0); // 注意：这里不需要 +1，因为我们处理的是二进制数据（包含nonce）
        ret = myrecv(sockfd, recv_buf.data(), len);
        if (ret <= 0)
        {
            cout << "Disconnected from server." << endl;
            exit(0);
        }

        string msg(recv_buf.data(), len);
        
        // 解密消息
        string decrypted = ChaCha20::decrypt(msg, KEY);
        
        cout << decrypted << endl;
    }
}

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

    cout << "Connect successful. You can start typing messages." << endl;

    // 启动发送线程和接收线程
    std::thread t1(sendThread, sockfd);
    std::thread t2(recvThread, sockfd);

    t1.join();
    t2.join();

    close(sockfd);
    return 0;
}
