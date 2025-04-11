//
// Created by ix on 25-4-7.
//
#pragma once
#include <nlohmann/json.hpp>
#include <utility>
#include <string>

enum MessageType{
    MessageText,
    MessageImage
};

enum CurrentState
{
    WAIT_FOR_HEADER_DESCRIPTOR = 0,
    WAIT_FOR_HEADER,
    WAIT_FOR_BODY,
};

enum UsedTo
{
    LOGIN = 0,
    CHAT = 1
};

#pragma pack(push,1)
struct S2C_Agreement
{
    uint32_t Id;
    uint32_t subPacketId;
    uint32_t residualByte;
    uint32_t totalPacketCount;
    uint32_t jsonSize;
};
#pragma pack(pop)


struct Chat_json
{
    std::string from;
    std::string fileName;
};

namespace ix {
namespace utility {

namespace JsonUtility {

    nlohmann::json GetJson(uint32_t json_size,std::vector<char>* stream);
    nlohmann::json GetJson(std::vector<char>* stream);
    uint32_t PacketHeader_descriptor(size_t size);
    // 修改后的 Chat 函数声明
    std::vector<char> Chat(UsedTo usedTo, std::string from, std::string type, std::string to, std::string fileName, size_t size);

    // 修改后的 Login 函数声明
    std::vector<char> Login(UsedTo usedTo, std::string userName, size_t name_size, std::string password, size_t password_size);


    template<UsedTo usedTo>
    struct FlagTag{};
    template<UsedTo usedTo,typename... Args>
    std::vector<char> PacketHeader(Args&&... args)
    {
        return Dispatch(FlagTag<usedTo>{},std::forward<Args>(args)...);
    }

    inline std::vector<char> Dispatch(FlagTag<CHAT>, std::string from, std::string type, std::string to, std::string fileName, size_t size)
    {
        return Chat(CHAT,from,type,to,fileName,size);
    }
    inline std::vector<char> Dispatch(FlagTag<LOGIN>, std::string userName, size_t name_size, std::string password, size_t password_size)
    {
        return Login(LOGIN,userName,name_size,password,password_size);

    }

    // template<UsedTo usedTo>
    // template<UsedTo usedTo,typename>

};

} // utility
} // ix

