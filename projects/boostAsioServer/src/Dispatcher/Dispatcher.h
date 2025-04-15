#pragma once
#include <nlohmann/adl_serializer.hpp>
#include <memory>

#include "../tcp_service/tcp_service.h"

namespace ix::dispatcher {

class Dispatcher {
protected:
    m_boost::Service::tcp_service* owner;
    Dispatcher() = default;
    virtual void handle_data(std::shared_ptr<std::vector<char>> data, nlohmann::json &&j) = 0;

public:
    virtual ~Dispatcher();
};

inline Dispatcher::~Dispatcher(){ owner = nullptr; };
} // Dispatcher
// ix
