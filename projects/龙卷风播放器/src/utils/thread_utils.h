//
// Created by ix on 25-5-25.
//
#pragma once
#include <memory>
#include <QThread>
#include <QDebug>

template<typename T,typename ...Args>
std::unique_ptr<T, std::function<void(T*)>> make_safe_thread(Args&&...args) {
    return std::unique_ptr<T, std::function<void(T*)>>(
        new T(std::forward<Args>(args)...),
        [](T* ptr) {
            if (ptr) {
                //ptr->quit();
                // if (!ptr->wait(3000)) {
                //     qWarning() << typeid(T).name() << " wait 超时，线程可能死锁了" << ptr;
                // }
                delete ptr;
            }
        }
    );
}


// ptr->requestInterruption();
// ptr->quit();