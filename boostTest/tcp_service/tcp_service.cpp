#include "tcp_service.h"

using namespace boost::asio;
using namespace std;
namespace ix::m_boost::Service {

tcp_service::tcp_service(std::string ip, unsigned short port,size_t thread_count) :
    id_increment(0),thread_count(thread_count), ip(ip),port(port),
    m_thread_pool(thread_count),m_work_guard(make_work_guard(m_io_context)){}



int tcp_service::get_id_increment()
{
    return id_increment++;
}
}
