#include <iostream>

#include "server.h"

#include <boost/filesystem.hpp>
#include <nlohmann/json.hpp>
using namespace boost::asio;
using namespace std;
namespace ix::m_boost::Service {

namespace fs = boost::filesystem;

Server::Server(std::string ip, unsigned short port,size_t thread_count) : tcp_service(ip,port,thread_count)
{
    load_user_table();
    init(ip, port);
    do_accept();
}
void Server::do_accept()
{
     cout << "监听中" << endl;
    m_acceptor->async_accept([this](::boost::system::error_code ec,ip::tcp::socket socket)
    {
        if (!ec)
        {
            int cur_id = id_increment++;
            //utility::Logger::Instance().Log(ix::utility::Logger::Level::DEBUG,__FILE__,__LINE__,"连接至服务器");
            sessions.emplace(cur_id,make_shared<ix::m_boost::session::Session_serv>(m_io_context,move(socket),cur_id));
            sessions[cur_id]->recv();
        }
        else
        {
            cout << "accept出错：" << ec.category().name() << "  " << ec.message() << endl;
        }
        do_accept();
    });
}
void Server::init(std::string ip, unsigned short port)
{
    ip::tcp::endpoint endpoint(ip::make_address(ip), port);

    try{
        m_acceptor->open(endpoint.protocol());
        m_acceptor->set_option(ip::tcp::acceptor::reuse_address(true));
        m_acceptor->bind(endpoint);
        m_acceptor->listen();
    }catch (const exception& e)
    {
        cout << "m_acceptor初始化错误 " << e.what() << endl;
    }
}

void Server::run()
{
    for (size_t i = 0; i < thread_count; i++)
    {
        ::boost::asio::post(m_thread_pool,[&]()
        {
            m_io_context.run();
        });
    }
    m_io_context.run();
}

optional<string> Server::verifyUserInfo(string username, string password)
{
    if (local_user_table.count(username) > 0 && local_user_table[username].password == password)
    {
        return local_user_table[username].nickname;
    }
    return nullopt;
}

void Server::transmitMessage(string from,string to,shared_ptr<vector<char>> context)
{
    //static const size_t MAX_MESSAGE_SIZE = 32 * 1024 * 1024;
    static const size_t MAX_FRAGMENT_SIZE = 1024 * 4;
    int headerSize = sizeof(S2C_agreement);
    nlohmann::json j;
    j["action"] = "login";
    j["from"] = from;
    S2C_agreement agreement;
    string str_j = j.dump();
    size_t totalSize = str_j.size() + context->size();
    size_t remainingSize =totalSize;
    agreement.jsonSize = str_j.size();

    agreement.MessageId = id_increment;
    agreement.FragmentsCount = (totalSize+MAX_FRAGMENT_SIZE) / MAX_FRAGMENT_SIZE;
    for (agreement.FragmentId = 0;agreement.FragmentId < agreement.FragmentsCount;agreement.FragmentId++,remainingSize -= agreement.thisSize)
    {
        agreement.thisSize = headerSize + min(remainingSize,MAX_FRAGMENT_SIZE);
        shared_ptr<vector<char>> message = make_shared<vector<char>>();
        const char* header = reinterpret_cast<const char*>(&agreement);
        auto header_v = vector<char>(header,header + headerSize);
        message->insert(message->begin(),header_v.begin(),header_v.end());
        message->insert(message->end(),context->begin(),context->begin()+agreement.thisSize);
        sessions[local_user_table[to].id]->send(message);
    }
    id_increment++;
}

// void Server::registered_user(std::string nickname, int id, std::string username, std::string password)
// {
//     sessions.emplace(nickname,make_shared<session::Session>());
// }

void Server::load_user_table()
{
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
        for (auto item : j)
        {
            local_user_table.emplace(j["name"],session::userInfo{
                j.value("id",-1),
                j.value("username","user"),
                j.value("password","123456"),
                j.value("nickname","soya"),
                false
            });
        }
    }catch (const exception& e)
    {
        cout << "读取本地用户表失败 ： " << e.what() << endl;
    }
}
}