#include "public.hpp"
#include <string.h>
#include <iostream>
#include <chrono>
#include <string>
#include <iomanip>
#include <sstream>
#include <sys/syscall.h>
#ifdef __APPLE__
#include <pthread.h>
#endif

pid_t get_tid()
{
#ifdef __APPLE__
    uint64_t tid;
    pthread_threadid_np(NULL, &tid);
    return (pid_t)tid;
#else
    return syscall(SYS_gettid);
#endif
}

bool hasNewlineAtEnd(const char *buffer)
{
    // 计算字符串的长度
    size_t length = strlen(buffer);
    // 检查长度是否大于0并且最后一个字符是否是换行符
    return length > 0 && buffer[length - 1] == '\n';
}

std::string getCurrentTimeInNanoseconds()
{
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 转换为 time_t 类型，用于获取日期部分
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);

    // 获取纳秒部分
    auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    auto fractional_seconds = now_ns.time_since_epoch().count() % 1000000000;

    // 格式化时间为字符串
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S");
    oss << "." << std::setfill('0') << std::setw(9) << fractional_seconds;

    return oss.str();
}

void xorEncryptDecrypt(char* data, size_t len, const std::string& key)
{
    if (key.empty()) return;
    for (size_t i = 0; i < len; ++i)
    {
        data[i] ^= key[i % key.length()];
    }
}

void xorEncryptDecrypt(std::string& data, const std::string& key)
{
    if (key.empty()) return;
    for (size_t i = 0; i < data.length(); ++i)
    {
        data[i] ^= key[i % key.length()];
    }
}