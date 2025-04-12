#include <iostream>

#include "client.h"
using namespace boost::asio;
using namespace std;
namespace ix::m_boost::Client {

Client::Client(std::string ip, unsigned short port,size_t thread_count) : tcp_service(ip,port,thread_count)
{
    init(ip, port);
}

void Client::connect(string ip,int port)
{
    ip::tcp::socket socket(m_io_context);
    int cur_id = get_id_increment();
    sessions.emplace(cur_id,make_shared<ix::m_boost::Service::Session>(m_io_context,move(socket),cur_id));

}

void Client::init(std::string ip, unsigned short port)
{

}

void Client::run()
{
    for (size_t i = 0; i < thread_count; i++)
    {
        ::boost::asio::post(m_thread_pool,[&]()
        {
            m_io_context.run();
        });
    }
    m_io_context.run();
}



}