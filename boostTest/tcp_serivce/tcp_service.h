#pragma once
#include <unordered_set>
#include <boost/asio.hpp>
#include "Session/Session.h"
namespace ix::m_boost::Service {

class tcp_service {
    // 计数器
    int id_increment;
    //线程数
    size_t thread_count;
    std::string ip;
    int port;
    ::boost::asio::thread_pool m_thread_pool;
    ::boost::asio::io_context m_io_context;
    ::boost::asio::io_context::work m_work_guard;

    ::boost::asio::ip::tcp::acceptor m_acceptor;
    std::unordered_map<int,Session> sessions;
public:
    explicit tcp_service(std::string ip, unsigned short port, size_t thread_count = 4);

    void do_accept();


    void post();
    void run();
private:
    void init(std::string ip, unsigned short port,size_t thread_count);


};

}
