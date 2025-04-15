//
// Created by ix on 25-4-12.
//

#include "ServerDispatcher.h"

#include <iostream>

#include "../server/server.h"
#include <nlohmann/json.hpp>
#include <string>

#include "../utility/JsonInterpreter/JsonInterpreter.h"
using namespace std;
namespace ix {
namespace dispatcher{
// Dispatcher
using json = nlohmann::json;
ServerDispatcher::ServerDispatcher(m_boost::Server::Server *Serv) : serv(Serv)
{
}

void ServerDispatcher::handle_data(std::shared_ptr<vector<char>> data, nlohmann::json&& j)
{
    std::string action = j["action"];
    if (action == "login")
    {

        if (optional<string> nickname = serv->verifyUserInfo(j["username"],j["password"]))
        {
            j["nickname"] = *nickname;
            j["confirm"] = true;
            serv->user_login(j["username"],j["from_id"]);
        }

        j["to_id"] = j["from_id"];
        j.erase("from_id");

        serv->transmitMessage(move(j),make_shared<vector<char>>());
    }
    else if (action == "chat")
    {
        //auto feedback = ix::utility::JsonInterpreter::feedback;
        nlohmann::json feedback;
        feedback["action"] = "feedback";
        feedback["from"] = "server";
        feedback["to_id"] = j["from_id"];
        feedback["condition"] = "success";
        string target = j["to"];
        cout << j.dump() << endl;
        optional<int> target_id = serv->get_id(target);
        if (target_id)
        {
            if (serv->is_online(target))
            {
                try
                {
                    j["to_id"] = *target_id;
                    // cout << *serv->get_user_nickname(j["from"]) << endl;
                    // j["from"] = *serv->get_user_nickname(j["from"]);
                    serv->transmitMessage(move(j),data);
                }catch (const exception& e)
                {
                    std::cout << e.what() << endl;
                }

            }
            else
            {
                feedback["condition"] = "failed";
                feedback["error"] = "对方未上线";
            }
        }
        else
        {
            feedback["condition"] = "failed";
            feedback["error"] = "未知对象";
        }
        //utility::JsonInterpreter::packaging(,);
        serv->transmitMessage(move(feedback),make_shared<vector<char>>());
    }
}
}
} // ix