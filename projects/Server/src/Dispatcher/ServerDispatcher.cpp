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
        int to_id = j["from_id"];
        j.erase("from_id");
        j["to_id"] = to_id;
        string username = j["username"].get<string>();

        if (optional<string> nickname = serv->verifyUserInfo(username,j["password"]))
        {
            j["nickname"] = *nickname;
            j["confirm"] = true;
            serv->user_login(
                to_id,
                j.value("username","error"),
                j.value("password","error"),
                j.value("nickname","unknown")
                );
            string q = "update users set status = 'ONLINE' where username = '" + username + "'";
            serv->Execute(q);
            //  发送房间信息

            json roomInfo = {
                {"action","rooms_receive"},
                {"to_id",to_id},
                {"room_names",serv->get_chatRoom_list()}
            };

            serv->transmitMessage(move(roomInfo),make_shared<vector<char>>());
            serv->on_user_log(username);
        }
        serv->transmitMessage(move(j),make_shared<vector<char>>());


    }
    else if (action == "disconnect")
    {
        serv->delete_user(j["id"]);
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
        //cout << q << endl;
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
            serv->on_chatRoom_receive_message_callback();
        }
    }
    else if (action == "log_out")
    {
        serv->handle_user_log_out(j["username"]);
    }
    else if (action == "request_chat_his_from_client")
    {
        cout << "request_chat_his_from_client" << endl;
        j["action"] = "request_chat_his_from_server";
        int to_id = j["from_id"];
        j.erase("from_id");
        j["to_id"] = to_id;
        if (auto res = serv->Query("select * from chatMessage where roomName = '" + j["request_name"].get<string>() + "'"))
        {
            shared_ptr<vector<char>> d = make_shared<vector<char>>();
            while (res->next())
            {
                string text(
                    "\n[" + res->getString("timestamp") + "]\n" +
                    res->getString("sender") + " 说：" +
                    res->getString("message") + "\n"
                );
                d->insert(d->end(),text.begin(),text.end());
            }
            //cout << string(d->begin(),d->end()) << endl;

            serv->transmitMessage(move(j),d);
        }
    }
    else if (action == "sign_in")
    {
        serv->sign_in_user(j["username"], j["password"],j["email"],j.value("nickname",""));
        json feedback = {
            {"action","sign_in"},
            {"confirm",true}
        };
        auto res = ix::utility::JsonInterpreter::packaging(move(feedback),{});
        serv->transmitMessage(res->first,res->second);
    }
}
}
} // ix