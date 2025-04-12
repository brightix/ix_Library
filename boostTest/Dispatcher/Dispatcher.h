#pragma once
#include <nlohmann/adl_serializer.hpp>
#include <memory>
namespace ix {
namespace dispatcher {

class Dispatcher {
protected:
    virtual void handle_data(std::shared_ptr<std::vector<char>> data,nlohmann::json j) = 0;
    virtual ~Dispatcher();
};

} // Dispatcher
} // ix
