#pragma once

#include <nlohmann/json.hpp>
#include <mysql/jdbc.h>
#include "../Dispatcher/ServerDispatcher.h"
#include "../tcp_service/tcp_service.h"
#include "../utility/SqlConnectionPool/SqlConnectionPool.h"


namespace ix::m_boost::Server {
class Server : public m_boost::Service::tcp_service{

    std::unique_ptr<dispatcher::ServerDispatcher> dispatcher;
    std::unordered_map<std::string,session::userInfo> local_user_table;
    sql::mysql::MySQL_Driver* sql_driver;
    std::vector<std::vector<std::string>> chatRoom_list;
    std::unordered_map<std::string,std::string> chatRoom_map;
public:
    Server();
    ~Server();
    void register_server(std::string ip, int port);

    void do_accept();

    void start_service() override;


    //void run() final;

    //注册用户s
    void registered_user(const std::string& nickname,int id,std::string username,std::string password);

    void load_user_table();

    bool is_online(std::string target);

    std::optional<int> get_id(std::string target);
    std::optional<std::string> get_user_nickname(std::string username);

    std::vector<std::vector<std::string>> get_chatRoom_list();

    void user_login(int id, std::string username, std::string password, std::string nickname);




    std::unique_ptr<sql::ResultSet> Query(const std::string &query);

    void Execute(const std::string &query);

    void delete_user(int id);

    void shutdown();


    void uploading_message(nlohmann::json &&j, std::shared_ptr<std::vector<char>> data);

    void sign_in_user(std::string username, std::string password, std::string email, std::string nickname="UNKNOWN");

    //检查登录信息
    std::optional<std::string> verifyUserInfo(std::string userName, std::string password);
//转发讯息
    void transmitMessage(nlohmann::json&& j, std::shared_ptr<std::vector<char>> context);

    void handle_user_log_out(std::string username);

    //std::unique_ptr<sql::Connection> sql_conn;
    std::unique_ptr<utility::SqlConnRALL> sql_conn;

    void on_user_log(std::string &name);
    void on_chatRoom_receive_message_callback();

    void set_user_log_callback(std::function<void(std::string&)> callback);
    void set_handle_chatRoom_receive_message_callback(std::function<void()> callback);
private:
    std::function<void(std::string&)> user_log_callback_;
    std::function<void()> handle_chatRoom_receive_message_callBack_;
};

}
