//
// Created by ix on 25-4-12.
//

#include "ServerDispatcher.h"
#include "../JsonInterpreter/JsonInterpreter.h"
#include <nlohmann/json.hpp>
#include <string>
using namespace std;
namespace ix {
namespace dispatcher{
// Dispatcher
using json = nlohmann::json;
ServerDispatcher::ServerDispatcher(m_boost::Service::Server *Serv) : serv(Serv)
{
}

void ServerDispatcher::handle_data(std::shared_ptr<vector<char>> data, nlohmann::json j)
{
    std::string action = j["action"];
    if (action == "login")
    {
        utility::JsonInterpreter::Confirm confirm = {"login","Server",serv->verifyUserInfo(j["userName"],j["password"]).has_value()};
        json send_j;
        utility::JsonInterpreter::to_json(j,confirm);
        string send_j_s = j.dump();
        serv->transmitMessage("Server",j["from"],make_shared<vector<char>>(send_j_s.begin(),send_j_s.end()));
    }
    else if (action == "chat")
    {
        serv->transmitMessage(j["from"],j["to"],data);
    }
}
}
} // ix