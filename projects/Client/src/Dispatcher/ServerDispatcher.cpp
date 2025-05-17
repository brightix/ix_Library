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
        string username = j["username"].get<string>();
        if (optional<string> nickname = serv->verifyUserInfo(username,j["password"]))
        {
            j["nickname"] = *nickname;
            j["confirm"] = true;
            serv->user_login(
                j.value("from_id",-1),
                j.value("username","error"),
                j.value("password","error"),
                j.value("nickname","unknown")
                );
            string q = "update users set status = 'ONLINE' where username = '" + username + "'";
            serv->Execute(q);
        }

        int to_id = j["from_id"];
        j.erase("from_id");
        j["to_id"] = to_id;


        serv->transmitMessage(move(j),make_shared<vector<char>>());

        //  发送房间信息

        json roomInfo = {
            {"action","rooms_receive"},
            {"to_id",to_id},
            {"room_names",serv->get_chatRoom_list()}
        };

        serv->transmitMessage(move(roomInfo),make_shared<vector<char>>());
        serv->on_user_connected(username);
    }
    else if (action == "disconnect")
    {
        serv->delete_user(j["id"]);
        string test = j.dump(4);
        cout << test << endl;
        string q = "update users set status = 'OFFLINE' where username = '" + j["username"].get<string>() + "'";
        try
        {
            serv->Execute(q);
        }
        catch (const exception& e)
        {
            cout << e.what() << endl;
        }
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
        //cout << j.dump() << endl;
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
    else if (action == "chat_to_room")
    {
        string q = "select * from chatRoomList where roomName = '" + j["roomName"].get<string>() + "'";
        cout << q << endl;
        auto res = serv->Query(q);
        if (res->next())
        {
            nlohmann::json feedback={
                {"action","feedback"},
                {"from","server"},
                {"to_id",j["from_id"]},
                {"condition","success"}
            };
            serv->transmitMessage(move(feedback),make_shared<vector<char>>());
            serv->uploading_message(move(j),data);
            serv->on_chatRoom_receive_message_callBack();
        }
    }
}
}
} // ix