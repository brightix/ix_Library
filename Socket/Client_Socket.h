#pragma once
#include "Socket.h"
namespace ix {
    namespace socket {
    class Client_Socket :
    public Socket
    {
    public:
        Client_Socket() = delete;
        Client_Socket(const std::string& ip,int port);
        ~Client_Socket() = delete;
    };
}
}


