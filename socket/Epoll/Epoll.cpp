//
// Created by ix on 25-4-8.
//

#include "Epoll.h"
#include <Logger.h>

#include <iostream>
#include <unistd.h>

#include <sys/epoll.h>
#include <sys/eventfd.h>

using namespace ix::socket;
using namespace std;

Epoll::Epoll()
{

}

bool Epoll::Epoll_init()
{
    epfd = epoll_create(1);
    if (epfd == -1) {
        perror("epoll_create");
        PRINT_ERRNO("epoll_create 错误");
    }
    curb_fd = eventfd(0,EFD_NONBLOCK);
    if (!epoll_control(curb_fd,EPOLL_CTL_ADD, EPOLLIN))
    {
        perror("curb_fd 创建失败");
        PRINT_ERRNO("curb_fd 创建失败");
        return false;
    }

    cout << "epfd创建成功" << endl;
    stop_event_fd = eventfd(0,EFD_NONBLOCK);
    epoll_control(stop_event_fd,EPOLL_CTL_ADD,EPOLLIN);
    return true;
}

int Epoll::Get_epfd() {
    return epfd;
}

bool Epoll::epoll_control(int fd,int op, int events)
{
    epoll_event ev{};
    ev.events = events;
    ev.data.fd = fd;
    return epoll_ctl(epfd,op,fd,&ev) != -1;
}

int Epoll::wait() {
    int ret;
    while (true)
    {
        ret = epoll_wait(epfd,evs,size,-1);
        if (ret == -1)
        {
            if (errno == EINTR)
            {
                // 被信号中断了，不是真的错，继续等待
                continue;
            }
            perror("epoll_wait 错误");
            break;
        }
        break;
    }
    return ret;
}