#pragma once

#include <nlohmann/json_fwd.hpp>
#include <nlohmann/json_fwd.hpp>

#include "../Dispatcher/ServerDispatcher.h"
#include "../tcp_service/tcp_service.h"
namespace ix::m_boost::Server {

class Server : public m_boost::Service::tcp_service{

    std::unique_ptr<dispatcher::ServerDispatcher> dispatcher;
    std::unordered_map<std::string,session::userInfo> local_user_table;
public:
    Server();

    void register_server(std::string ip, int port);

    void do_accept();

    void start_service() override;


    void run() final;

    //注册用户
    void registered_user(std::string nickname,int id,std::string username,std::string password);

    void load_user_table();

    bool is_online(std::string target);

    std::optional<int> get_id(std::string target);

    void user_login(std::string username, int id);

    std::optional<std::string> get_user_nickname(std::string username);

    //检查登录信息
    std::optional<std::string> verifyUserInfo(std::string userName, std::string password);
//转发讯息
    void transmitMessage(nlohmann::json&& j, std::shared_ptr<std::vector<char>> context);
private:

};

}
