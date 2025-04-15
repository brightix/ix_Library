//
// Created by ix on 25-4-13.
//

#include "JsonInterpreter.h"
#include <iostream>
namespace ix {
namespace utility::JsonInterpreter {
using namespace std;

optional<pair<size_t,shared_ptr<vector<char>>>> packaging(unordered_map<string,std::variant<string,int,bool>>&& json_map,vector<char>&& context)
{
    nlohmann::json j;
    for (auto [key,val] : json_map)
    {
        if (holds_alternative<string>(val))
        {
            j[key] = get<string>(val);
        }
        else if (holds_alternative<int>(val))
        {
            j[key] = get<int>(val);
        }
        else if (holds_alternative<bool>(val))
        {
            j[key] = get<bool>(val);
        }
        else
        {
            cout << "packaging 非法数据" << endl;
            return std::nullopt;
        }
    }
    string j_s = j.dump();
    cout << j_s << endl;
    shared_ptr<vector<char>> totalMsg = make_shared<vector<char>>(j_s.begin(),j_s.end());
    totalMsg->insert(totalMsg->end(),context.begin(),context.end());
    return make_pair(j_s.size(),move(totalMsg));
}


} // utility
} // ix