//
// Created by ix on 25-4-11.
//
#include <iostream>


#include "Logger/Logger.h"
#include "tcp_service.h"
using boost::asio::ip::tcp;
using namespace ix::m_boost;
using namespace std;
namespace ix::m_boost::Service {

tcp_service::tcp_service(std::string ip, unsigned short port,size_t thread_count) : ip(ip),port(port), thread_count(thread_count),m_thread_pool(thread_count),
    m_work_guard(m_io_context),m_acceptor(m_io_context),id_increment(0)
{
    tcp::endpoint endpoint(::boost::asio::ip::make_address(ip), port);
    m_acceptor.open(endpoint.protocol());
    m_acceptor.set_option(tcp::acceptor::reuse_address(true));
    m_acceptor.bind(endpoint);
    m_acceptor.listen();
    init(ip, port, thread_count);
    do_accept();
}
void tcp_service::do_accept()
{
    m_acceptor.async_accept([this](::boost::system::error_code ec,tcp::socket socket)
    {
        if (!ec)
        {
            int cur_id = id_increment++;
            debug(string(to_string(cur_id) + "连接至服务器").data());
            sessions.emplace(cur_id,Session(m_io_context,move(socket),cur_id));
            sessions[cur_id].do_recv();
        }
        else
        {
            cout << "accept出错：" << ec.category().name() << "  " << ec.message() << endl;
        }
        do_accept();
    });
}
void tcp_service::init(std::string ip, unsigned short port,size_t thread_count)
{

}

void tcp_service::run()
{
    for (size_t i = 0; i < thread_count; i++)
    {
        ::boost::asio::post(m_thread_pool,[&]()
        {
            m_io_context.run();
        });
    }
}



}