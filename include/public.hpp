#pragma once
#include <string>
#include <unistd.h>

pid_t get_tid();

bool hasNewlineAtEnd(const char *buffer);
/*
以字符串的形式返回纳秒级别的时间点
*/
std::string getCurrentTimeInNanoseconds();
