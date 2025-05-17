//
// Created by ix on 25-4-13.


#include "JsonInterpreter.h"
#include <iostream>
namespace ix {
namespace utility::JsonInterpreter {
using namespace std;

using json = nlohmann::json;

optional<pair<size_t,shared_ptr<vector<char>>>> packaging_login(string username,string password,string time)
{
    if (username.empty())
    {
        return nullopt;
    }
    if (password.empty())
    {
        return nullopt;
    }

    json j = {
        {"action","login"},
        {"username",username},
        {"password",password},
        {"nickname",string{}},
        {"time",time},
        {"confirm",false}
    };
    string j_s = j.dump();
    shared_ptr<vector<char>> totalMsg = make_shared<vector<char>>(j_s.begin(),j_s.end());
    return make_pair(j_s.size(),move(totalMsg));
}

optional<pair<size_t,shared_ptr<vector<char>>>> packaging_recv_chat_his(string roomName)
{
    if (roomName.empty())
    {
        return nullopt;
    }
    json j = {
        {"action","recv_chat_his"}
    };
    string j_s = j.dump();
    shared_ptr<vector<char>> totalMsg = make_shared<vector<char>>(j_s.begin(),j_s.end());
    return make_pair(j_s.size(),move(totalMsg));
}

optional<pair<size_t,shared_ptr<vector<char>>>> packaging(json&& j,shared_ptr<vector<char>> context)
{
    string j_s = j.dump();
    //cout << j_s << endl;
    shared_ptr<vector<char>> totalMsg = make_shared<vector<char>>(j_s.begin(),j_s.end());
    if (context)
    {
        totalMsg->insert(totalMsg->end(),context->begin(),context->end());
    }
    return make_pair(j_s.size(),totalMsg);
}


} // utility
} // ix