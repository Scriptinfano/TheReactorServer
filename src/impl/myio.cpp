#include "myio.hpp"
#include "log.hpp"
#include <iostream>
using namespace std;
ssize_t myrecv(int sockfd, void *buf, size_t n)
{
    ssize_t nread;
    if (nread = recv(sockfd, buf, n, 0) <= 0)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("recv() failed or connection closed:").c_str());
        exit(-1);
    }
    return nread;
}
ssize_t mysend(int sockfd, const void *buf, size_t n){
    ssize_t nread;
    if (nread=send(sockfd, buf, n, 0) <= 0)
    {
        logger.logMessage(FATAL, __FILE__, __LINE__, logger.createErrorMessage("send() failed").c_str());
        exit(-1);
    }
    return nread;
}