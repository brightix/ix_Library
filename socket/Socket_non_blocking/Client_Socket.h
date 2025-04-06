#pragma once
#include "Socket.h"
namespace ix {
    namespace socket {
    class Client_Socket :
    public Socket
    {
        bool isConnected;
    public:
        Client_Socket() = delete;
        Client_Socket(const std::string& ip,int port);
        explicit Client_Socket(int file_describe);
        bool GetConnectStatus();
        ~Client_Socket() = default;
    };
}
}


