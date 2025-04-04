#include "Client_Socket.h"
using namespace ix::socket;
Client_Socket::Client_Socket(const std::string& ip, int port)
{
	//set_non_blocking();
	set_recv_buffer(10 * 1024);
	set_send_buffer(10 * 1024);
	set_linger(true, 0);
	set_keepalive();
	set_reuseAddr();
	isConnected = connect(ip,port);
}

bool ix::socket::Client_Socket::GetConnectStatus()
{
	return isConnected;
}
