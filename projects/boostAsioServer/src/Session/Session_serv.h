#pragma once
#include "Session.h"
namespace ix {
namespace m_boost {
namespace session {

class Session_serv : public Session{
public:
    Session_serv(boost::asio::io_context& io_context,boost::asio::ip::tcp::socket&& socket,int id);
private:
    //void handle_login(nlohmann::json j, std::shared_ptr<std::vector<char>> data) override;
    //void handle_chat(nlohmann::json j, std::shared_ptr<std::vector<char>> data) override;
};

} // Session
} // m_boost
} // ix
