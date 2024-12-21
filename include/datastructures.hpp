/*
自定义数据结构的定义文件
*/
#pragma once
#include <iostream>
using namespace std;
/*
基于数组实现的循环队列
*/
template <class T, int MAXLENGTH>
class CircularQueue
{
private:
    /* data */
    bool initiated;//队列被初始化的标志
    T data[MAXLENGTH];//实际存放数据的数组
    int head;//头指针
    int tail;//尾指针
    int length;//队列实际长度
    CircularQueue(const CircularQueue &) = delete;//禁用拷贝构造函数
    CircularQueue &operator=(const CircularQueue &) = delete;//禁用赋值运算符

    
public:
    CircularQueue();
    void init();
    /*
    从队尾加入一个元素，如果已经满了，则会返回false
    */
    bool push(const T &elem);

    int size();
    bool isEmpty();
    bool isFull();
    /*
    查看队头元素的值，但是不出队
    */
    T &front();
    bool pop(T*elem);
    void print();

};

template <class T, int MAXLENGTH>
void CircularQueue<T, MAXLENGTH>::init()
{
    // 循环队列的初始化只能有一次
    if (initiated != true)
    {
        head = 0;
        tail = 0;
        length = 0;
        memset(data, 0, sizeof(data));
        initiated = true;
    }
}

template <class T, int MAXLENGTH>
CircularQueue<T, MAXLENGTH>::CircularQueue() : initiated(false)
{
    init();
}

template <class T, int MAXLENGTH>
bool CircularQueue<T, MAXLENGTH>::push(const T &elem)
{
    if (isFull())
    {
        cout << "循环队列已满, 入队失败" << endl;
        return false;
    }
    data[tail] = elem;
    tail = (tail + 1) % MAXLENGTH;
    length++;
    return true;
}

template <class T, int MAXLENGTH>
int CircularQueue<T, MAXLENGTH>::size()
{
    return length;
}

template <class T, int MAXLENGTH>
bool CircularQueue<T, MAXLENGTH>::isEmpty()
{
    return length == 0;
}

template <class T, int MAXLENGTH>
bool CircularQueue<T, MAXLENGTH>::isFull()
{
    return (length == MAXLENGTH);
}

template <class T, int MAXLENGTH>
T &CircularQueue<T, MAXLENGTH>::front()
{
    return data[head];
}

template <class T, int MAXLENGTH>
bool CircularQueue<T, MAXLENGTH>::pop(T *elem)
{
    if (isEmpty())
        return false;
    *elem = data[head];
    head = (head + 1) % MAXLENGTH;
    length--;
    return true;
}

template <class T, int MAXLENGTH>
void CircularQueue<T, MAXLENGTH>::print()
{
    int ptr = head;
    do 
    {
        cout << data[ptr] << " " << endl;
        ptr = (ptr + 1) % MAXLENGTH;
    } while (ptr != tail);
}