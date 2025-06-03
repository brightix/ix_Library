//
// Created by ix on 25-5-21.
//
#include "RingBuffer.h"

#include <iostream>
using namespace std;

size_t RingBuffer_size_t::read(uint8_t* data, size_t requested)
{
    std::unique_lock<mutex> lock(mutex_);
    empty.wait(lock,[this]
    {
        return size_ > 0;
    });
    size_t to_read = min(requested,size_);
    for (size_t i = 0;i < to_read;i++)
    {
        data[i] = buf_[head_];
        head_ = (head_+1) % buf_.size();
    }
    size_ -= to_read;

    lock.unlock();
    full.notify_one();
    return to_read;

}

size_t RingBuffer_size_t::write(const uint8_t* d, size_t length)
{
    size_t written = 0;
    while (written < length)
    {
        unique_lock<mutex> lock(mutex_);
        full.wait(lock,[this]
        {
            return buf_.size() > size_;
        });
        size_t capacity_this_time = min(buf_.size() - size_,length-written);
        length -= capacity_this_time;
        for (size_t i = 0; i < capacity_this_time; i++, written++)
        {
            buf_[tail_] = d[written];
            tail_ = (tail_ + 1) % buf_.size();
            size_++;
        }
        lock.unlock();
        not_full.notify_one();
    }
    return written;
}

size_t RingBuffer_size_t::size()
{
    return size_;
}

RingBuffer_size_t::RingBuffer_size_t(size_t size): tail_(0), head_(0), size_(0)
{
    buf_.resize(size);
}





