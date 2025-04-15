#include <iostream>

#include <boost/asio.hpp>

#include "src/Logger/Logger.h"
#include "src/server/server.h"
using namespace boost::asio;
using ip::tcp;
using namespace std;
int main()
{
    ix::m_boost::Server::Server server;
    ix::utility::Logger::Instance().Open("Service.log");
    //server.switch_thread_solution(4);

    try
    {
        string ip = "127.0.0.1";
        int port = 8080;

        server.register_server(ip,port);
        server.start_service();
        server.run();
    }catch (const exception& e)
    {
        cout << "service出错" << e.what() << endl;
    }
    return 0;
}