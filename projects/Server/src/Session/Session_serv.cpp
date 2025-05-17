#include "Session_serv.h"

#include <nlohmann/json.hpp>
using namespace std;
namespace ix {
namespace m_boost {
namespace session {
} // Session
session::Session_serv::Session_serv(boost::asio::io_context &io_context, boost::asio::ip::tcp::socket &&socket, int id) : Session(io_context,move(socket),id)
{
}

void session::Session_serv::handle_login(nlohmann::json j, std::shared_ptr<vector<char>> data)
{

}


} // m_boost
} // ix