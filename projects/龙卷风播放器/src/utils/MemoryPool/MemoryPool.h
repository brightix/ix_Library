//
// Created by ix on 25-5-20.
//
#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H
#include <functional>
#include <memory>
#include <queue>

template<typename T>
class MemoryPool {
    using Storage = typename std::aligned_storage<sizeof(T),alignof(T)>::type;
    std::queue<Storage*> pool_;
    int size;
    std::mutex mutex;
public:
    explicit MemoryPool(int size=4)
    {
        this->size = size;
        allocate_memo();
    }

    template<typename ...Args>
    std::unique_ptr<T,std::function<void(T*)>> get(Args&&... args)
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (pool_.empty())
        {
            allocate_memo();
        }
        Storage* raw = pool_.front();
        pool_.pop();
        T* obj = new(raw) T(std::forward<Args>(args)...);
        return std::unique_ptr<T,std::function<void(T*)>>(obj,[this,raw](T* ptr)
        {
            ptr->~T();
            std::lock_guard<std::mutex> lock(mutex);
            this->pool_.emplace(raw);
        });
    }
    void return_obj(T* ptr)
    {
        std::lock_guard<std::mutex> lock(mutex);
        ptr->~T();
        this->pool_.emplace(reinterpret_cast<Storage*>(ptr));
    }

    ~MemoryPool()
    {
        while (!pool_.empty())
        {
            ::operator delete(pool_.front());
            pool_.pop();
        }
    }

    //空闲或者合适的时候手动调用
    void release_memo()
    {
        std::lock_guard<std::mutex> lock(mutex);
        if (pool_.size() > size / 2)
        {
            for (int i = 0; i < size; i++)
            {
                operator delete(pool_.front());
                pool_.pop();
            }
        }
    }
private:
    void allocate_memo()
    {
        for (int i = 0; i < size; i++)
        {
            pool_.emplace(reinterpret_cast<Storage*>(::operator new(sizeof(Storage))));
        }
        size *= 2;
    }
};


template<typename T>
class MemoryPool_shared {
    struct Slot {
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
        bool constructed = false;
    };

    std::queue<Slot*> pool_;
    int size;
    bool thread_safe = false;

    // 可选：线程安全
    std::mutex mtx;

public:
    explicit MemoryPool_shared(int init_size = 4, bool thread_safe = false)
        : size(init_size), thread_safe(thread_safe)
    {
        allocate_memo();
    }

    template<typename ...Args>
    std::shared_ptr<T> get(Args&&... args)
    {
        if (thread_safe) std::lock_guard<std::mutex> lock(mtx);

        if (pool_.empty())
        {
            allocate_memo();
        }

        Slot* slot = pool_.front();
        pool_.pop();
        T* obj = nullptr;

        try {
            obj = new(&slot->storage) T(std::forward<Args>(args)...);
            slot->constructed = true;

            auto deleter = [this, slot](T* ptr) {
                ptr->~T();
                slot->constructed = false;
                if (thread_safe) {
                    std::lock_guard<std::mutex> lock(mtx);
                    pool_.emplace(slot);
                } else {
                    pool_.emplace(slot);
                }
            };
            return std::shared_ptr<T>(obj, deleter);
        } catch (...) {
            pool_.emplace(slot); // 失败回收
            throw;
        }
    }

    // 手动归还（用于非shared_ptr使用场景）
    void return_obj(T* ptr)
    {
        Slot* slot = reinterpret_cast<Slot*>(
            reinterpret_cast<char*>(ptr) - offsetof(Slot, storage));
        ptr->~T();
        slot->constructed = false;

        if (thread_safe) {
            std::lock_guard<std::mutex> lock(mtx);
            pool_.emplace(slot);
        } else {
            pool_.emplace(slot);
        }
    }

    ~MemoryPool_shared()
    {
        while (!pool_.empty())
        {
            Slot* slot = pool_.front();
            pool_.pop();
            if (slot->constructed)
            {
                reinterpret_cast<T*>(&slot->storage)->~T();
            }
            ::operator delete(slot);
        }
    }

    // 手动释放部分内存
    void release_memo()
    {
        if (thread_safe) std::lock_guard<std::mutex> lock(mtx);

        int count = static_cast<int>(pool_.size()) - size / 2;
        for (int i = 0; i < count && !pool_.empty(); ++i)
        {
            Slot* slot = pool_.front();
            pool_.pop();
            if (slot->constructed)
            {
                reinterpret_cast<T*>(&slot->storage)->~T();
            }
            ::operator delete(slot);
        }
    }

private:
    void allocate_memo()
    {
        for (int i = 0; i < size; ++i)
        {
            pool_.emplace(static_cast<Slot*>(::operator new(sizeof(Slot))));
        }
        size *= 2;
    }
};





#endif //MEMORYPOOL_H
