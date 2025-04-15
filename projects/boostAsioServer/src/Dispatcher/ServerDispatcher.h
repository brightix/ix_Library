#pragma once
#include "Dispatcher.h"



namespace ix::m_boost::Server {
class Server;  // ✅ 前向声明
}

namespace ix::dispatcher {

class ServerDispatcher : public dispatcher::Dispatcher {
    ::ix::m_boost::Server::Server* serv;
public:
    explicit ServerDispatcher(::ix::m_boost::Server::Server* Serv);
    void handle_data(std::shared_ptr<std::vector<char>> data, nlohmann::json &&j) override;
    ~ServerDispatcher() override= default;
};

} // Dispatcher
// ix
