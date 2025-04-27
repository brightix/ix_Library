#pragma once
#include "Dispatcher.h"



namespace ix::m_boost::Client {
class Client;
}

namespace ix::dispatcher {

class ClientDispatcher : public dispatcher::Dispatcher {
    ::ix::m_boost::Client::Client* clt;
public:
    explicit ClientDispatcher(::ix::m_boost::Client::Client* clt);
    void handle_data(std::shared_ptr<std::vector<char>> data, nlohmann::json &&j) override;
    ~ClientDispatcher() override= default;
};

} // Dispatcher
// ix
