#include "buffer.hpp"
#include "log.hpp"
Buffer::Buffer()
{
}
Buffer::~Buffer()
{
}
void Buffer::append(const char *data, size_t size)
{
    buf_.append(data, size); // append(const char* s, size_t n)：将指针 s 指向的字符数组中的前 n 个字符附加到当前字符串末尾
}
size_t Buffer::getSize()
{
    return buf_.size();
}
const char *Buffer::getData()
{
    return buf_.data();
}
void Buffer::clear()
{
    return buf_.clear();
}

std::string Buffer::getString()
{
    return buf_;
}

void Buffer::erase(size_t pos, size_t n)
{
    buf_.erase(pos, n);
}

void Buffer::appendWithHead(std::string data)
{
    size_t size = data.size();
    logger.logMessage(DEBUG, __FILE__, __LINE__, "已经进入appendWithHead,数据为%s，大小为%d", data.c_str(), data.size());
    buf_.append((char *)&size, sizeof(int)); // 添加数据头
    buf_.append(data);                       // 添加实际数据
    logger.logMessage(DEBUG, __FILE__, __LINE__, "appendWithHead调用完成,此时自定义缓冲区中的数据为%s", buf_.c_str());
}