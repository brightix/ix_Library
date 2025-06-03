//
// Created by ix on 25-5-21.
//
#ifndef RINGBUFFER_H
#define RINGBUFFER_H
#include <condition_variable>
#include <deque>
#include <mutex>
#include <queue>
#include <variant>
#include <vector>
#include <bits/shared_ptr_atomic.h>


template<typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity): size_(0), capacity(capacity)
    {
        stopped.store(false);
    }
    bool read(T& data)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        full.wait(lock,[this]
        {
            return size_ > 0 || stopped;
        });
        if (size_ == 0 && stopped) {
            return false; // 表示结束，调用者退出线程
        }
        data = std::move(buf_.front());
        buf_.pop();
        size_--;
        lock.unlock();
        not_full.notify_one();
        return true;
    }

    bool write(T&& d)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full.wait(lock,[this]
        {
            return capacity > size_;
        });
        if (capacity <= size_ && stopped) {
            return false; // 表示结束，调用者退出线程
        }
        buf_.emplace(std::move(d));
        size_++;
        lock.unlock();
        full.notify_one();
        return true;
    }
    size_t size() const { return size_; }

    void stop()
    {
        stopped.store(true);
        full.notify_one();
        not_full.notify_one();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        while (!buf_.empty())
        {
            buf_.pop();
            size_--;
        }
    }
private:
    std::mutex mutex_;
    std::condition_variable not_full,full;
    std::atomic<bool> stopped = {false};
    size_t size_;
    std::queue<T> buf_;
    size_t capacity;
};


class RingBuffer_size_t {
public:
    RingBuffer_size_t(size_t size);
    size_t read(uint8_t *data, size_t requested);
    size_t write(const uint8_t *data, size_t length);

    size_t size();

private:

    std::mutex mutex_;
    std::condition_variable not_full,full;

    size_t tail_;
    size_t head_;
    size_t size_;
    std::vector<uint8_t> buf_;
};

#endif //RINGBUFFER_H
