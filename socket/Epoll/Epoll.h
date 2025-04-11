//
// Created by ix on 25-4-8.
//
#pragma once
#include <sys/epoll.h>



namespace ix::socket
{

class Epoll
{
public:
    int Get_epfd();// 获取epoll套接字
protected:
    Epoll();

    bool Epoll_init();

    bool epoll_control(int fd,int op,int events);// 设置epoll 状态
    int wait();// 等待接收连接请求或数据


    int curb_fd;
    int epfd;
    size_t size = 1024;
    struct epoll_event evs[1024]{};

    int stop_event_fd;
};

}
