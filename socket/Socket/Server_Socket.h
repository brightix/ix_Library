#pragma once
#include "Socket.h"
#include "Client_Socket.h"
#include <memory>
#include <unordered_map>
#include <condition_variable>
namespace ix {
    namespace socket {
class Server_Socket :
    public Socket
{
public:
    Server_Socket(const std::string& ip, int port);
    Server_Socket() = delete;


    /* 选择 I/O多路复用 方法 */
        void init_epoll();
        int Get_epfd();// 获取epoll套接字
        epoll_event* Get_evs();// 获取事件列表
        bool epoll_control(int fd,int op,struct epoll_event&& ev);// 设置epoll 状态
        int wait();// 等待接收连接请求或数据
        void register_client();// 注册用户
        std::string handle_data(int fd);

    void print_all_client();

    void print_clients_periodically();

    void send_to_client(int cfd, std::string data);

    void modify_client_event(int fd);

    void close_client(int fd);

    ~Server_Socket() override ;

private:

    // 客户端连接
    std::unordered_map<int, std::unique_ptr<Client_Socket>> clients;
    // epoll
    int epfd;

    size_t size;
    struct epoll_event evs[1024];

    //打印的互斥锁
    std::mutex clients_mutex;
    std::mutex print_mutex;
    std::condition_variable print_condition;
    bool printNow = false;
};
    }
}

