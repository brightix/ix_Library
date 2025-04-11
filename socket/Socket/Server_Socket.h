#pragma once
#include "Socket.h"
#include "Client_Socket.h"
#include "../Epoll/Epoll.h"

#include <memory>
#include <unordered_map>
#include <condition_variable>
#include <queue>

struct ClientSendTask
{
    std::mutex mtx;
    std::queue<std::vector<char>> sendQueue;
    bool sending = false;
};





struct ClientDataStore
{
    int currentState = WAIT_FOR_HEADER_DESCRIPTOR;
    int usedTo;
    uint32_t nextSize = 4;
    std::vector<char> data;
    nlohmann::json header;
};

struct ClientInfo
{
    std::string userName;
    std::string password;
    bool isOnline;
    int fd = -1;
};

namespace ix {
    namespace socket {

class Server_Socket :
    public Socket, public Epoll
{
public:
    Server_Socket(const std::string& ip, int port);

    //查询用户信息
    bool is_user_online(std::string& userName);
    int get_user_fd(std::string &userName);
    int has_user_in_local_table(std::string &userName);
    bool check_user_password(std::string &userName, std::string &password);
    //加载缓存用户
    void loadLocalUser();

    Server_Socket() = delete;


    int register_client();// 注册用户
    void recv_to_buffer(int fd);

    std::vector<char> handle_data(int fd,size_t totalSize);

    bool checkClientRegistrationStatus(int cfd);

    void print_all_client();

    void print_clients_periodically();

    void send_to_client(int cfd, std::vector<char> data);

    void forward_message(int fd, ClientDataStore& cds);

    void queueSend_new(int fd, std::vector<char> header,std::vector<char> data);
    void queueSend(int fd, std::vector<char>& data);

    void send_next(int fd);

    void modify_client_event(int cfd);

    void close_client(int cfd);

    int name_to_fd(std::string& userName);

    //注册用户表
    void loadUser(int fd, std::string &name);
    void loadUser(int fd, std::vector<char> name);


    bool check_userName(std::string userName);

    void shut_down();

    void run();





    void handle_epoll_event(int requestFd);

    ~Server_Socket() override ;

private:
// 客户端连接
    //缓冲区
    std::unordered_map<int,std::vector<char>> recvBuffer;
    //单次最大数据包
    size_t MaxPacketSize = 1024;

    std::atomic<uint32_t> PACKET_ID = 0;

    //本地用户表
    std::unordered_map<std::string, ClientInfo> local_clients;



    std::unordered_map<int, std::unique_ptr<Client_Socket>> clients;//在线用户表
    std::unordered_map<int,std::string> fd_name;//用户名和描述符映射表
    std::unordered_map<std::string,int> name_fd;//用户名和描述符映射表
    std::unordered_map<int,ClientSendTask> clientSendTasks;//发送给用户的任务进度表
    std::unordered_map<int,ClientDataStore> clientsData;//用户发来的数据

    std::mutex clients_mutex;
};
    }
}

