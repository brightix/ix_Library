#pragma once
#include <string>
#include "../tcp_service/tcp_service.h"
namespace ix::m_boost::Client {

class Client : public m_boost::Service::tcp_service{
public:
    Client(std::string ip, unsigned short port, size_t thread_count = 4);

    void connect(std::string ip, int port);

    void run() override final;
private:
    void init(std::string ip, unsigned short port) override final;
};

}