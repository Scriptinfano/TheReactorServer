#pragma once
#include<string>
class Buffer{
private:
    std::string buf_;//用于存放数据

public:
    Buffer();
    ~Buffer();
    /*
    将size大小的数据追加到buffer中
    */
    void append(const char *data, size_t size);

    /*
    也是将数据加到buf_中，只不过会先计算数据部分的大小，然后将大小放在报头中，然后将数据放在报头后面
    */
    void appendWithHead(std::string data);

    /*
    获取缓冲区目前存放的数据的大小
    */
    size_t getSize();
    /*
    获取缓冲区的数据的首地址
    */
    const char *getData();
    /*
    清空缓冲区
    */
    void clear();
    /*
    返回内部string对象
    */
    std::string getString();
    /*
    @param 从指定的位置之后删除指定数量的字节
    @param pos 要删除的起始字节
    @param n 要从指定位置之后删除多少个字节
    */
    void erase(size_t pos, size_t n);
};