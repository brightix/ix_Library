#include "ClientDispatcher.h"

#include <iostream>
#include <nlohmann/json.hpp>
#include "../client/client.h"

namespace ix {
namespace dispatcher {
using namespace std;
ClientDispatcher::ClientDispatcher(ix::m_boost::Client::Client *clt) : clt(clt)
{

}
void ClientDispatcher::handle_data(std::shared_ptr<std::vector<char>> data, nlohmann::json&& j)
{
    std::string action = j["action"];
    if (action == "login")
    {
        if (j["confirm"])
        {
            //登录成功
            clt->login_success(move(j));
        }
        else
        {
            //登录失败
            clt->login_failed();
        }
        clt->on_login_callback();
    }
    else if (action == "chat")
    {
        clt->handle_chat(move(j),data);
    }
    else if (action == "feedback")
    {
        if (j["condition"] == "success")
        {
            cout << "数据发送成功" << endl;
        }
        else
        {
            cout << "数据发送失败" << endl;
            cout << "错误： " << j["error"] << endl;
        }
    }
    else if (action == "recv_chat_his")
    {
        string s(data->begin(),data->end());
        clt->on_recv_chat_his_callback(s);
    }
    else if (action == "rooms_receive")
    {
        vector<vector<string>> r = j.value("room_names",vector<vector<string>>{});
        clt->set_chatRoom_list(r);
        clt->on_recv_chat_list_callback();
    }
    else if (action == "request_chat_his_from_server")
    {
        string d(data->begin(),data->end());
        clt->on_recv_chat_his_callback(d);
    }
    else if (action == "disconnect")
    {
        //clt->disconnect(j["id"]);
        clt->disconnect();
    }
    else if (action == "ping")
    {
        clt->on_end_ping(j.value("start",-999));
    }

}
} // dispatcher
} // ix