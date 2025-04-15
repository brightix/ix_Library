#pragma once

#include <string>
#include <variant>
#include <nlohmann/json.hpp>

namespace ix {
namespace utility::JsonInterpreter {

using json = nlohmann::json;

// struct RequestLogin
// {
//     std::string action;
//     std::string username;
//     std::string password;
//     bool confirm;
//     std::string time;
// };

inline std::unordered_map<std::string,std::variant<std::string,int,bool>> RequestLogin = {
    {"action","login"},
    {"from",std::string{}},
    {"username",std::string{}},
    {"password",std::string{}},
    {"confirm",false},
    {"nickname",std::string{}},
    {"time",std::string{}}
};

inline std::unordered_map<std::string,std::variant<std::string,int,bool>> Chat = {
    {"action","chat"},
    {"from",std::string{}},
    {"to",std::string{}},
    {"time",std::string{}},
};

inline std::unordered_map<std::string,std::variant<std::string,int,bool>> feedback = {
    {"action","feedback"},
    {"from",std::string{}},
    {"to",std::string{}},
    {"error",std::string{}},
    {"condition",std::string{}},
    {"time",std::string{}},
};

std::optional<std::pair<size_t,std::shared_ptr<std::vector<char>>>> packaging(std::unordered_map<std::string,std::variant<std::string,int,bool>>&& json_map,std::vector<char>&& context);


// struct Confirm
// {
//     std::string action;
//     std::string from;
//     bool val;
//     std::string time;
// };

// struct Chat
// {
//     std::string action;
//     std::string from;
//     std::string to;
// };
//
//
// template<typename T>
// void from_json(const json& j, T& stc);
//
// template<>
// inline void from_json<Confirm>(const json& j, Confirm& stc)
// {
//     j.at("action").get_to(stc.action);
//     j.at("from").get_to(stc.from);
//     j.at("val").get_to(stc.val);
// }
//
// template<>
// inline void from_json<RequestLogin>(const json& j, RequestLogin& stc)
// {
//     j.at("action").get_to(stc.action);
//     j.at("username").get_to(stc.username);
//     j.at("password").get_to(stc.password);
// }
//
// template<>
// inline void from_json<Chat>(const json& j, Chat& stc)
// {
//     j.at("action").get_to(stc.action);
//     j.at("from").get_to(stc.from);
//     j.at("to").get_to(stc.to);
// }
//
// template<typename T>
// void to_json(json& j,T stc);
//
// template<>
// inline void to_json(json& j,RequestLogin& stc)
// {
//     j = json{
//         {"action",stc.action},
//         {"username",stc.username},
//         {"password",stc.password},
//     };
// }
//
// inline void to_json(json& j,Confirm& stc)
// {
//     j = json{
//             {"action",stc.action},
//             {"from",stc.from},
//             {"val",stc.val},
//         };
// }
//
// template<>
// inline void to_json(json& j,Chat& stc)
// {
//     j = json{
//             {"action",stc.action},
//             {"from",stc.from},
//             {"to",stc.to},
//         };
// }

} // utility
} // ix
