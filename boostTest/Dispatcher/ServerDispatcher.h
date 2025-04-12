#pragma once
#include "Dispatcher.h"
#include "../server/server.h"

namespace ix {
namespace dispatcher {

class ServerDispatcher : public dispatcher::Dispatcher {
    m_boost::Service::Server* serv;
public:
    ServerDispatcher(m_boost::Service::Server* Serv);
    void handle_data(std::shared_ptr<std::vector<char>> data, nlohmann::json j) override;
    ~ServerDispatcher() = default;
};

} // Dispatcher
} // ix
