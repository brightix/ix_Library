#include "Socket.h"
#include "../../utility/Logger/Logger.h"

#include <iostream>
#include <string>
#include <cstring>


#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>

using namespace ix::socket;

using namespace std;

Socket::Socket() : m_ip(""), m_port(0), m_socket(-1)
{
	m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == -1) {
		error("socket creat failed!");
		return;
	}
	debug("socket creat success!");
}

bool Socket::bind(const std::string& ip, const int port)
{
	struct sockaddr_in sockaddr{};
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	if (ip.empty()) {
		sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else {
		if (inet_pton(AF_INET, ip.c_str(), &sockaddr.sin_addr) != 1) {
			error(ix::utility::Logger::Instance().PrintErrno("IP_addr_transform").c_str());
			return false;
		}
	}

	if (::bind(m_socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
		error(ix::utility::Logger::Instance().PrintErrno("socket_bind").c_str());
		return false;
	}

	m_ip = ip;
	m_port = port;
	debug("socket bind success: ip=%s,port=%d", ip.c_str(), port);
	return true;
}

bool Socket::listen(int backlog)
{
	if (::listen(m_socket, backlog) < 0) {
		error(ix::utility::Logger::Instance().PrintErrno("listen").c_str());
		return false;
	}
	debug("socket listenning...");
	return true;
}

bool Socket::connect(const std::string& ip, const int port)
{
	struct sockaddr_in sockaddr;
	std::memset(&sockaddr, 0, sizeof(sockaddr_in));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	if (inet_pton(AF_INET, ip.c_str(), &sockaddr.sin_addr) != 1) {
		error("IP transform failed");
		return false;
	}
	if (::connect(m_socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == -1) {
		error(ix::utility::Logger::Instance().PrintErrno("socket connect").c_str());
		return false;
	}
	m_ip = ip;
	m_port = port;
	return true;
}

int Socket::accept()
{
	int connfd = ::accept(m_socket, nullptr, nullptr);
	if (connfd == -1) {
		error(ix::utility::Logger::Instance().PrintErrno("link describe").c_str());
		return -1;
	}
	debug("link describe success!");
	return connfd;
}

int Socket::send(const char* buf, size_t len)
{
	return ::send(m_socket, buf, len, MSG_NOSIGNAL);
}

int ix::socket::Socket::recv(char* buf, size_t len)
{
	int ret = ::recv(m_socket, buf, len, 0);
	if (ret == -1) {
		//error(ix::utility::Logger::Instance().PrintErrno("recv").c_str());
	}
	return ret;
}

void ix::socket::Socket::close()
{
	if (m_socket > 0) {
		::close(m_socket);
		m_socket = 0;
	}
}

int Socket::Get_fd()
{
	return m_socket;
}

bool ix::socket::Socket::set_non_blocking()
{
    int flags = fcntl(m_socket,F_GETFL,0);
    if(flags == -1){
    	error(ix::utility::Logger::Instance().PrintErrno("fcntl_F_GETFL").c_str());

        return false;
    }
    flags |= O_NONBLOCK;
	if (fcntl(m_socket,F_SETFL,flags) == -1) {
		error(ix::utility::Logger::Instance().PrintErrno("fcntl_F_SETFL").c_str());

		return false;
	}
	return true;
}

bool ix::socket::Socket::set_send_buffer(int size)
{
	if (setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size)) == -1) {
		error(ix::utility::Logger::Instance().PrintErrno("set_send_buffer").c_str());
		return false;
	}
	return true;
}

bool ix::socket::Socket::set_recv_buffer(int size)
{
	if (setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size)) == -1) {
		error(ix::utility::Logger::Instance().PrintErrno("set_recv_buffer").c_str());
		return false;
	}
	return true;
}

bool ix::socket::Socket::set_linger(bool active, int seconds)
{
	struct linger l;
	memset(&l, 0, sizeof(l));
	l.l_onoff = static_cast<u_short>(active);
	l.l_linger = static_cast<u_short>(seconds);
	if (setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (const char*)&l, sizeof(l))) {
		error(ix::utility::Logger::Instance().PrintErrno("set_linger").c_str());
		return false;
	}
	return true;
}

bool ix::socket::Socket::set_keepalive()
{
	int flag = 1;
	if (setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&flag, sizeof(flag))) {
		error(ix::utility::Logger::Instance().PrintErrno("set_keepalive").c_str());
		return false;
	}
	return true;
}

bool ix::socket::Socket::set_reuseAddr()
{
	int flag = 1;
	if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(flag))) {
		error(ix::utility::Logger::Instance().PrintErrno("set_reuseAddr").c_str());
		return false;
	}
	return true;
}



ix::socket::Socket::Socket(int socket) : m_ip(""), m_port(0), m_socket(socket)
{

}

Socket::~Socket()
{
	close();
}