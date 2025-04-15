#pragma once
#include <boost/asio.hpp>
#include <memory>
#include "../Session/Session.h"
#include "../Logger/Logger.h"

namespace ix::m_boost::Service {

class tcp_service {
protected:
    // 计数器
    std::atomic<int> id_increment;

    //线程数
    size_t thread_count;

    std::thread m_io_thread;

    std::mutex sessions_mtx;
    std::string ip;
    int port;

    ::boost::asio::io_context m_io_context;
    std::unique_ptr<::boost::asio::thread_pool> m_thread_pool;
    ::boost::asio::executor_work_guard<::boost::asio::io_context::executor_type> m_work_guard;

    std::optional<::boost::asio::ip::tcp::acceptor> m_acceptor;
    std::unordered_map<int,std::shared_ptr<ix::m_boost::session::Session>> sessions;
    std::unordered_map<int,std::string> id_name;
public:
    //这只是提供tcp服务，必须手动注册连接
    tcp_service();
    int get_id_increment();

    void start_threadpool();

    bool switch_thread_solution(size_t count);

    virtual void run() = 0;

    virtual void start_service() = 0;
    virtual ~tcp_service();
protected:
    void join_all_thread();

    S2C_agreement handle_htonl(S2C_agreement agreement);

    void handle_ntohl(S2C_agreement &agreement);

    void init(std::string ip, unsigned short port);
};

}
