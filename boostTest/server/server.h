#pragma once

//#include "../Logger/Logger.h"
#include "../tcp_service/tcp_service.h"
#include "../Session/Session_serv.h"
namespace ix::m_boost::Service {

class Server : public m_boost::Service::tcp_service{


    std::unordered_map<std::string,session::userInfo> local_user_table;
public:
    explicit Server(std::string ip, unsigned short port, size_t thread_count = 4);

    void do_accept();


    void run() override final;

//注册用户
    void registered_user(std::string nickname,int id,std::string username,std::string password);

    void load_user_table();

    //检查登录信息
    std::optional<std::string> verifyUserInfo(std::string userName, std::string password);
//转发讯息
    void transmitMessage(std::string from, std::string to, std::shared_ptr<std::vector<char>> context);
    ~Server() = default;
private:
    void init(std::string ip, unsigned short port) override final;


};

}
