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
}
} // dispatcher
} // ix