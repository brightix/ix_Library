#include <iostream>

#include "client.h"

#include "../utility/JsonInterpreter/JsonInterpreter.h"
#include "../Dispatcher/ClientDispatcher.h"
using namespace boost::asio;
using namespace std;
namespace ix::m_boost::Client {

Client::Client() : tcp_service(),dispatcher(make_unique<dispatcher::ClientDispatcher>(this))
{
    is_connected = false;
}

void Client::start_service()
{
    switch_thread_solution(4);
}

void Client::connect(string ip,int port)
{
    ip::tcp::resolver resolver(m_io_context);
    auto endPoints = resolver.resolve(ip,to_string(port));
    auto socket = make_shared<ip::tcp::socket>(m_io_context);
    cout << "已提交连接服务器申请" << endl;

     boost::asio::async_connect(*socket,endPoints,[this,socket](boost::system::error_code ec,ip::tcp::endpoint)
     {
         if (!ec)
         {
             cout << "连接成功" << endl;
             std::lock_guard<std::mutex> lock(sessions_mtx);
             int cur_id = get_id_increment();
             sessions.emplace(cur_id,make_shared<session::Session>(m_io_context,socket,cur_id));
             sessions[cur_id]->set_data_handler([this](shared_ptr<vector<char>> data,nlohmann::json&& j)
             {
                 this->dispatcher->handle_data(data,move(j));
             });
             sessions[cur_id]->do_read();

             is_connected = true;
         }
         else
         {
             cout << "Exception " << ec.message() << endl;
         }
     });
}



void Client::run()
{
    while (true)
        {
            cout << "输入指令 : ";
            string order;
            getline(cin,order);
            if (order == "1")//"connect")
            {
                // cout << "请输入ip port : ";
                // string ip;
                // int port;
                // cin >> ip;
                // cin >> port;
                // cin.ignore();
                string default_ip = "127.0.0.1";
                int default_port = 8080;
                connect(default_ip,default_port);
            }
            else if (has_connect())
            {
                if (order == "send")
                {
                    string to;
                    cout << "to: ";getline(cin,to);

                    string context;
                    cout << "context: ";getline(cin,context);

                    unordered_map<string, std::variant<string, int, bool>> json_map = ix::utility::JsonInterpreter::Chat;
                    json_map.at("from") = get_usr_info().username;
                    json_map.at("to") = to;
                    optional<pair<size_t,shared_ptr<vector<char>>>> totalMsg = ix::utility::JsonInterpreter::packaging(move(json_map),vector<char>(context.begin(),context.end()));

                    //cout << totalMsg->first << "  " << totalMsg->second->size() << endl;
                    if (totalMsg)
                    {
                        send_to(totalMsg->first,totalMsg->second);
                    }
                }
                else if (order == "login")
                {
                    string username;
                    cout << "username: ";getline(cin,username);

                    string password;
                    cout << "password: ";getline(cin,password);

                    unordered_map<string, std::variant<string, int, bool>> json_map = ix::utility::JsonInterpreter::RequestLogin;
                    json_map.at("from") = "requestLogin";
                    json_map.at("username") = username;
                    json_map.at("password") = password;
                    optional<pair<size_t,shared_ptr<vector<char>>>> totalMsg = utility::JsonInterpreter::packaging(move(json_map),vector<char>());
                    if (totalMsg)
                    {
                        send_to(totalMsg->first,totalMsg->second);
                    }
                }
                else if (order == "exit")
                {
                    cout << "退出" << endl;
                    break;
                }
            }
            else
            {
                cout << "你还没有连接至服务器" << endl;
            }
        }
}

bool Client::has_connect()
{
    return is_connected;
}
//解偶，只负责传输协议和内容
void Client::send_to(size_t jsonSize,shared_ptr<vector<char>> msg)
{
    nlohmann::json j;
    static const size_t MAX_FRAGMENT_SIZE = 1024 * 4;
    int headerSize = sizeof(S2C_agreement);

    S2C_agreement agreement;
    size_t totalSize = msg->size();
    size_t remainingSize = totalSize;
    agreement.jsonSize = jsonSize;

    agreement.MessageId = get_id_increment();
    agreement.FragmentsCount = (totalSize+MAX_FRAGMENT_SIZE) / MAX_FRAGMENT_SIZE;
    for (agreement.FragmentId = 0;agreement.FragmentId < agreement.FragmentsCount;agreement.FragmentId++,remainingSize -= agreement.thisSize)
    {
        size_t packetSize = min(remainingSize,MAX_FRAGMENT_SIZE);
        agreement.thisSize = packetSize;
        shared_ptr<vector<char>> message = make_shared<vector<char>>();
        auto htonl_agreement = handle_htonl(agreement);
        const char* header = reinterpret_cast<const char*>(&htonl_agreement);
        auto header_v = vector<char>(header,header + headerSize);
        message->insert(message->begin(),header_v.begin(),header_v.end());
        message->insert(message->end(),msg->begin(),msg->begin()+packetSize);
        //cout << "send_to:" << string(message->begin(),message->end()) << " end" << endl;
        sessions[0]->send(message);
    }
    cout << agreement.MessageId << " 包已发送至缓冲区" << endl;
}


LocalUserInfo Client::get_usr_info()
{
    return local_user_info;
}

void Client::login_success(nlohmann::json&& j)
{
    local_user_info.nickname = j["nickname"];
    local_user_info.username = j["username"];
    local_user_info.is_logining = true;
    cout << "登录成功，欢迎你 " << local_user_info.nickname << endl;

}

void Client::login_failed()
{
    cout << "登录失败，帐号或密码错误" << endl;
}

void Client::handle_chat(nlohmann::json&& j,shared_ptr<vector<char>> data)
{
    //cout << j.dump(4) << endl;
    auto message = string(data->begin(),data->end());
    cout << j.value("from","unknown") << ":" << message << endl;
}

}