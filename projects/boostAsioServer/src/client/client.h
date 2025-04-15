#pragma once
#include <string>
#include <nlohmann/json.hpp>

#include "../tcp_service/tcp_service.h"
#include "../Session/Session.h"

#include "../Dispatcher/ClientDispatcher.h"

namespace ix::m_boost::Client {
struct LocalUserInfo
{
    std::string nickname;
    std::string username;
    std::string password;
    bool is_logining;
    //std::string username;
};
class Client : public m_boost::Service::tcp_service{

    std::unique_ptr<dispatcher::ClientDispatcher> dispatcher;
    bool is_connected;
    LocalUserInfo local_user_info;
public:
    Client();

    void start_service() override;

    void connect(std::string ip, int port);

    void run() final;

    bool has_connect();

    void send_to(size_t jsonSize, std::shared_ptr<std::vector<char>> msg);

    LocalUserInfo get_usr_info();

    void login_success(nlohmann::json &&j);

    void login_failed();

    void handle_chat(nlohmann::json && j, std::shared_ptr<std::vector<char>> data);

private:

};

}
