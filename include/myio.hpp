#pragma once
#include <sys/socket.h>
ssize_t myrecv(int sockfd, void *buf, size_t n);
ssize_t mysend(int sockfd, const void *buf, size_t n);