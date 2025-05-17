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
    bool is_logined;
    int cur_id;
    //std::string username;
};
class Client : public m_boost::Service::tcp_service{

    std::unique_ptr<dispatcher::ClientDispatcher> dispatcher;
    bool is_connected;
    bool is_listed;
    LocalUserInfo local_user_info;
    std::vector<std::vector<std::string>> chat_lists;

public:
    Client();

    void start_service() override;

    void connect(std::string ip, int port);

    void run() final;

    bool has_connect();
    bool has_logined();

    void Ping();

    void send_to(size_t jsonSize, std::shared_ptr<std::vector<char>> msg);

    LocalUserInfo get_usr_info();

    void login_success(nlohmann::json &&j);

    void login_failed();

    void handle_chat(nlohmann::json && j, std::shared_ptr<std::vector<char>> data);
    void set_chatRoom_list(std::vector<std::vector<std::string>> &room_list);

    //设置连接状态检查
    void set_on_connect(std::function<void()> &&callback);
    void on_connect_callback();

    //设置登陆检查
    void set_on_login(std::function<void()> &&callback);
    void on_login_callback();

    //设置ping值
    void set_on_end_ping_callback(std::function<void(int)> callback);
    void on_end_ping(long long t);

    void set_on_recv_chat_list_callback(std::function<void()> &&callback);
    void on_recv_chat_list_callback();

    void set_on_recv_chat_his_callback(std::function<void(std::string&)> &&callback);

    void on_recv_chat_his_callback(std::string &s);

    void disconnect();

    std::vector<std::vector<std::string>> get_chat_lists();

    void log_out();

    void request_chat_his(std::string &name);

    void init_local_user_info();

private:    //  callback
    std::function<void()> connect_callback;
    std::function<void()> login_callback;
    std::function<void(int)> end_ping_callback;
    std::function<void()> recv_chat_list_callback;
    std::function<void(std::string&)> recv_chat_his_callback;
};

}
