#pragma once
#include "Socket.h"
namespace ix {
    namespace socket {
class Server_Socket :
    public Socket
{
public:
    Server_Socket(const std::string& ip, int port);
    Server_Socket() = delete;

    ~Server_Socket() = default;
};
    }
}

