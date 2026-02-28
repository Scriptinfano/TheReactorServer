#pragma once
#include <string>
#include <unistd.h>

pid_t get_tid();

bool hasNewlineAtEnd(const char *buffer);
/*
以字符串的形式返回纳秒级别的时间点
*/
std::string getCurrentTimeInNanoseconds();

/*
使用简单的异或加密/解密
*/
void xorEncryptDecrypt(char* data, size_t len, const std::string& key);
void xorEncryptDecrypt(std::string& data, const std::string& key);
