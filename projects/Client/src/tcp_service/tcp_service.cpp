#include "tcp_service.h"

#include <iostream>

using namespace boost::asio;
using namespace std;
namespace ix::m_boost::Service {

tcp_service::tcp_service() :
    id_increment(0),thread_count(3), ip(""),port(-1),
    m_work_guard(make_work_guard(m_io_context))
{
    // m_io_thread = move(thread([this]()
    // {
    //     cout << m_io_context.run() << endl;
    // }));
}

void tcp_service::init(std::string ip, unsigned short port)
{
    this->ip = ip;
    this->port = port;
}


int tcp_service::get_id_increment()
{
    return id_increment++;
}

void tcp_service::start_threadpool()
{
    m_thread_pool = make_unique<::boost::asio::thread_pool>(thread_count);
    for (size_t i = 0; i < thread_count; i++)
    {
        try
        {
            post(*m_thread_pool,[&]()
            {
                m_io_context.run();
            });
        }catch (const exception& e)
        {
            cout << e.what() << endl;
        }
    }
}

bool tcp_service::switch_thread_solution(size_t count)
{
    if (count == thread_count || count > 8)
    {
        cout << "线程设置未生效" << endl;
        return false;
    }
    if (thread_count != 0)
    {
        join_all_thread();
        cout << "已回收所有线程" << endl;
    }
    thread_count = count;
    start_threadpool();
    return true;
}

void tcp_service::run()
{
}

tcp_service::~tcp_service()
{
    // if (m_io_thread.joinable())
    // {
    //     m_io_thread.join();
    // }
    m_io_context.stop();
    join_all_thread();
}

void tcp_service::join_all_thread()
{
    if (m_thread_pool)
    {
        cout << "等待所有线程回归" << endl;
        m_thread_pool->join();
    }
}

S2C_agreement tcp_service::handle_htonl(S2C_agreement agreement)
{
    agreement.jsonSize = htonl(agreement.jsonSize);
    agreement.MessageId = htonl(agreement.MessageId);
    agreement.FragmentsCount = htonl(agreement.FragmentsCount);
    agreement.FragmentId = htonl(agreement.FragmentId);
    agreement.thisSize = htonl(agreement.thisSize);
    return agreement;
}

}
