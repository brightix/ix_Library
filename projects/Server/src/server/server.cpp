#include <iostream>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "server.h"

#include "../utility/JsonInterpreter/JsonInterpreter.h"
#include "../utility/SqlConnectionPool/SqlConnectionPool.h"

using namespace boost::asio;
using namespace std;
namespace ix::m_boost::Server {

namespace fs = boost::filesystem;

Server::Server() : tcp_service(),dispatcher(make_unique<dispatcher::ServerDispatcher>(this))
{
    //load_user_table();
    //init_sql();
    start_threadpool();
    sql_conn = make_unique<utility::SqlConnRALL>(utility::SqlConnectionPool::Instance());
    auto res = sql_conn->query("select * from chatRoomList");
    while (res->next())
    {
        chatRoom_list.emplace_back(vector<string>{
            "room",
            res->getString("roomName"),
            to_string(res->getInt("headCount")),
            res->getString("lastRecv")
        });
    }
}

Server::~Server()
{
    Execute("update users set status = 'OFFLINE'");
    //cout << "将所有用户设置为离线" << endl;
}

void Server::register_server(string ip, int port)
{
    init(ip,port);
    cout << "服务器已启动，ip = " << ip << ", port = " << port << endl;
}
void Server::do_accept()
{
    cout << "监听中" << endl;
    m_acceptor->async_accept([this](const ::boost::system::error_code &ec,ip::tcp::socket socket)
    {
        if (!ec)
        {
            int cur_id = id_increment++;
            //utility::Logger::Instance().Log(ix::utility::Logger::Level::DEBUG,__FILE__,__LINE__,"连接至服务器");
            sessions.emplace(cur_id,make_shared<ix::m_boost::session::Session>(m_io_context,make_shared<ip::tcp::socket>(move(socket)),cur_id));
            sessions[cur_id]->set_data_handler([this](shared_ptr<vector<char>> data, nlohmann::json j)
                {
                    dispatcher->handle_data(move(data),move(j));
                });

            sessions[cur_id]->do_read();

            cout << cur_id << " 连接成功" << endl;
        }
        else
        {
            cout << "accept出错：" << ec.category().name() << "  " << ec.message() << endl;
        }
        do_accept();
    });
}
void Server::start_service()
{
    init(ip,port);
    m_acceptor = move(ip::tcp::acceptor(m_io_context, ip::tcp::endpoint(ip::tcp::v4(), port)));
    cout << "服务器初始化成功" << endl;
    do_accept();
}
//
// void Server::run()
// {
//     while (true)
//     {
//         cout << "输入指令 : ";
//         string order;
//         getline(cin,order);
//         if (order == "exit")//"connect")
//         {
//             cout << "退出" << endl;
//             break;
//         }
//     }
// }



optional<string> Server::verifyUserInfo(string username, string password)
{
    cout << "验证用户" << endl;
    utility::SqlConnRALL conn_rall(ix::utility::SqlConnectionPool::Instance());
    unique_ptr<sql::ResultSet> res = conn_rall.query("Select username , password ,nickname from local_user_info");
    while (res->next())
    {
        if (res->getString("username") == username && res->getString("password") == password)
        {
            return res->getString("nickname");
        }
    }
    return nullopt;
}

void Server::transmitMessage(nlohmann::json&& j,shared_ptr<vector<char>> context)
{

    //static const size_t MAX_MESSAGE_SIZE = 32 * 1024 * 1024;
    int to_session_id = j["to_id"];
    j.erase("to_id");

    static const size_t MAX_FRAGMENT_SIZE = 1024 * 4;
    int headerSize = sizeof(S2C_agreement);
    S2C_agreement agreement;
    string str_j = j.dump();

    size_t totalSize = str_j.size() + context->size();
    size_t remainingSize =totalSize;
    agreement.jsonSize = str_j.size();
    agreement.MessageId = get_id_increment();
    agreement.FragmentsCount = (totalSize+MAX_FRAGMENT_SIZE) / MAX_FRAGMENT_SIZE;

    shared_ptr<vector<char>> totalMsg = make_shared<vector<char>>(str_j.begin(),str_j.end());
    totalMsg->insert(totalMsg->end(),context->begin(),context->end());
    for (agreement.FragmentId = 0;agreement.FragmentId < agreement.FragmentsCount;agreement.FragmentId++,remainingSize -= agreement.thisSize - headerSize)
    {
        agreement.thisSize = min(remainingSize,MAX_FRAGMENT_SIZE);
        shared_ptr<vector<char>> message = make_shared<vector<char>>();

        auto htonl_agreement = handle_htonl(agreement);
        const char* header = reinterpret_cast<const char*>(&htonl_agreement);
        auto header_v = vector<char>(header,header + headerSize);

        message->insert(message->begin(),header_v.begin(),header_v.end());

        message->insert(message->end(),totalMsg->begin(),totalMsg->begin()+agreement.thisSize);
        sessions[to_session_id]->send(message);
    }
    cout << agreement.MessageId << " 包已发送至缓冲区" << endl;
}

void Server::handle_user_log_out(std::string username)
{
    if (auto res = Query("select username from users where username ='" + username + "'"))
    {
        Execute("update users set status = 'OFFLINE' where username ='" + username + "'");
        string u;
        on_user_log(u);
    }
}

// void Server::registered_user(const std::string& nickname,const std::string& username, const std::string& password)
// {
//     sessions.emplace(nickname,make_shared<session::Session>());
// }

void Server::load_user_table()
{
    cout << fs::current_path() << endl;

    fs::path path("user_table.txt");

    if (!fs::exists(path))
    {
        cout << "文件不存在" << endl;
        return;
    }
    std::ifstream file(path.string());
    if (!file.is_open())
    {
        cout << "文件打开失败" << endl;
        return;
    }
    nlohmann::json j;

    try
    {
        file >> j;
        for (auto item : j["user_table"])
        {
            local_user_table.emplace(item["username"],session::userInfo{
                item.value("id",-1),
                item.value("username","user"),
                item.value("password","123456"),
                item.value("nickname","soya"),
                false
            });
        }
    }catch (const exception& e)
    {
        cout << "读取本地用户表失败 ： " << e.what() << endl;
    }
}

bool Server::is_online(string target)
{
    return local_user_table[target].isOnline;
}

optional<int> Server::get_id(string target)
{
    return local_user_table.contains(target) ? optional<int>(local_user_table[target].id) : nullopt;
}

void Server::user_login(int id,string username,string password,string nickname)
{
    local_user_table[username].isOnline = true;
    local_user_table[username].id = id;
    sessions[id]->update_info(id,username,password,nickname);
}

optional<string> Server::get_user_nickname(string username)
{
    return local_user_table[username].nickname;
}

std::vector<std::vector<std::string>> Server::get_chatRoom_list()
{
    return chatRoom_list;
}

// void Server::init_sql()
// {
//     try{
//         sql_driver = sql::mysql::get_driver_instance();
//         sql_conn = unique_ptr<sql::Connection>(sql_driver->connect("tcp://127.0.0.1:3306","root","123"));
//         sql_conn->setSchema("my_database");
//     }catch (const exception& e)
//     {
//         cout << "数据库初始化失败 ：" << e.what() << endl;
//     }
// }
std::unique_ptr<sql::ResultSet> Server::Query(const string &query)
{
    return sql_conn->query(query);
}

void Server::Execute(const std::string& query) {
    sql_conn->execute(query); // 用于 INSERT/UPDATE/DELETE
}

void Server::delete_user(int id)
{
    sessions.erase(id);
}

void Server::shutdown()
{
    m_io_context.stop();
}

void Server::set_user_log_callback(std::function<void(string&)> callback)
{
    user_log_callback_ = callback;
}

void Server::set_handle_chatRoom_receive_message_callback(std::function<void()> callback)
{
    handle_chatRoom_receive_message_callBack_ = move(callback);
}

void Server::on_user_log(string& name)
{
    if (user_log_callback_)
    {
        user_log_callback_(name);
    }
}

void Server::on_chatRoom_receive_message_callback()
{
    if (handle_chatRoom_receive_message_callBack_)
    {
        handle_chatRoom_receive_message_callBack_();
    }
}

void Server::uploading_message(nlohmann::json&& j,shared_ptr<vector<char>> data)
{
    string message(data->begin(),data->end());

     string roomName(j["roomName"].get<string>());
     string sender(j["nickname"].get<string>());

    string t = "'" + roomName+ "','" + sender + "','" + message + "'";
    Execute("insert into chatMessage(roomName,sender,message) values(" + t + ")");


}

void Server::sign_in_user(string username,string password,string email,string nickname)
{
    if (username.empty() || password.empty() || email.empty())
    Execute("insert into users(username,password,email,nickname) values('" +
        username + "','" +
        password + "','" +
        email + "','" +
        nickname +
        "')");
}

}