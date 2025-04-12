#pragma once
#include <boost/asio.hpp>
#include <memory>
#include "../Session/Session.h"
#include "../Logger/Logger.h"
namespace ix::m_boost::Service {

class tcp_service {
protected:
    // 计数器
    int id_increment;
    //线程数
    size_t thread_count;
    std::string ip;
    int port;
    ::boost::asio::thread_pool m_thread_pool;
    ::boost::asio::io_context m_io_context;
    ::boost::asio::executor_work_guard<::boost::asio::io_context::executor_type> m_work_guard;

    std::optional<::boost::asio::ip::tcp::acceptor> m_acceptor;
    std::unordered_map<int,std::shared_ptr<ix::m_boost::session::Session>> sessions;
    std::unordered_map<int,std::string> id_name;
public:
    explicit tcp_service(std::string ip, unsigned short port, size_t thread_count = 4);


    int get_id_increment();

    virtual void run() = 0;
    virtual ~tcp_service();
protected:
    virtual void init(std::string ip, unsigned short port) = 0;
};

}
