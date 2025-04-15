#include <iostream>
#include <boost/asio.hpp>
#include "src/client/client.h"
#include "src/utility/JsonInterpreter/JsonInterpreter.h"
using namespace std;
using namespace boost::asio::ip;

void input(ix::m_boost::Client::Client& client);
int main()
{
    try
    {
        ix::m_boost::Client::Client client;
        client.start_service();
        client.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
