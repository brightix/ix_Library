#include <iostream>
#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include "server.h"
#include "../utility/SqlConnectionPool/SqlConnectionPool.h"

using namespace boost::asio;
using namespace std;
namespace ix::m_boost::Server {

namespace fs = boost::filesystem;

Server::Server() : tcp_service(),dispatcher(make_unique<dispatcher::ServerDispatcher>(this))
{
    load_user_table();
    init_sql();
}

void Server::register_server(string ip, int port)
{
    init(ip,port);
    cout << "服务器已启动，ip = " << ip << ", port = " << port << endl;
}
void Server::do_accept()
{
     cout << "监听中" << endl;
     //auto socket = make_shared<boost::asio::ip::tcp::socket>(m_acceptor->accept());

    // boost::system::error_code ec;

    //
    // if (!ec)
    // {
    //     int cur_id = id_increment++;
    //     //utility::Logger::Instance().Log(ix::utility::Logger::Level::DEBUG,__FILE__,__LINE__,"连接至服务器");
    //     sessions.emplace(cur_id,make_shared<ix::m_boost::session::Session>(m_io_context,socket,cur_id));
    //     sessions[cur_id]->do_read();
    //     cout << cur_id << " 连接成功" << endl;
    // }
    // else
    // {
    //     cout << "accept出错：" << ec.category().name() << "  " << ec.message() << endl;
    // }
    // do_accept();

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
    ip::tcp::endpoint endpoint(ip::make_address(ip), port);
    m_acceptor = move(ip::tcp::acceptor(m_io_context, ip::tcp::endpoint(ip::tcp::v4(), port)));
    // try{
    //     m_acceptor->open(endpoint.protocol());
    //     m_acceptor->set_option(ip::tcp::acceptor::reuse_address(true));
    //     m_acceptor->bind(endpoint);
    //     m_acceptor->listen();
    // }catch (const exception& e)
    // {
    //     cout << "m_acceptor初始化错误 " << e.what() << endl;
    //     return;
    // }
    cout << "服务器初始化成功" << endl;
    do_accept();
}

void Server::run()
{
    m_io_context.run();
}



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

void Server::user_login(string username,int id)
{
    local_user_table[username].isOnline = true;
    local_user_table[username].id = id;
}

optional<string> Server::get_user_nickname(string username)
{
    return local_user_table[username].nickname;
}

void Server::init_sql()
{
    try{
        sql_driver = sql::mysql::get_driver_instance();
        sql_conn = unique_ptr<sql::Connection>(sql_driver->connect("tcp://127.0.0.1:3306","root","123"));
        sql_conn->setSchema("my_database");
    }catch (const exception& e)
    {
        cout << "数据库初始化失败 ：" << e.what() << endl;
    }
}
}