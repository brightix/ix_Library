#include "Socket.h"
#include <winsock2.h>
#include <iostream>
#include "Logger.h"
#pragma comment(lib,"ws2_32.lib")
using namespace ix::socket;

using namespace std;

void log_socket_error(const std::string& context);
Socket::Socket() : m_ip(""), m_port(0), m_socket(0)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fatal("initial failed!");
	}
	m_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == INVALID_SOCKET) {
		error("socket creat failed!");
		return;
	}
	debug("socket creat success!");
}

bool Socket::bind(const std::string& ip, const int port)
{
	struct sockaddr_in sockaddr;

	std::memset(&sockaddr, 0, sizeof(sockaddr_in));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	if (ip.empty()) {
		sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	else {
		if (InetPtonA(AF_INET, ip.c_str(), &sockaddr.sin_addr) != 1) {
			log_socket_error("IP addr transform failed");
			return false;
		}
	}

	if (::bind(m_socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) < 0) {
		log_socket_error("bind");
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
		log_socket_error("listen");
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
	if (InetPtonA(AF_INET, ip.c_str(), &sockaddr.sin_addr) != 1) {
		error("IP µØÖ·×ª»»Ê§°Ü£¬´íÎóÂë£º%d", WSAGetLastError());
		return false;
	}
	if (::connect(m_socket, (struct sockaddr*)&sockaddr, sizeof(sockaddr)) == SOCKET_ERROR) {
		error("socket connect error: error=%d,errmsg=%s", errno, ix::utility::Logger::Instance().PrintErrno(errno).c_str());
		return false;
	}
	m_ip = ip;
	m_port = port;
	return true;
}

SOCKET Socket::accept()
{
	SOCKET connfd = ::accept(m_socket, nullptr, nullptr);
	if (connfd == INVALID_SOCKET) {
		log_socket_error("link describe");
		return -1;
	}
	debug("link describe success!");
	return connfd;
}

int Socket::send(const char* buf, int len)
{
	int ret = ::send(m_socket, buf, len, 0);
	if (ret == SOCKET_ERROR) {
		log_socket_error("send");
	}
	return ret;
}

int ix::socket::Socket::recv(char* buf, int len)
{
	return ::recv(m_socket, buf, len, 0);
}

void ix::socket::Socket::close()
{
	if (m_socket > 0) {
		::closesocket(m_socket);
		m_socket = 0;
	}
}

bool ix::socket::Socket::set_non_blocking()
{
	u_long mode = 1;
	if (ioctlsocket(m_socket, FIONBIO, &mode)) {
		log_socket_error("set_non_blocking");
		return false;
	}
	return true;
}

bool ix::socket::Socket::set_send_buffer(int size)
{
	if (setsockopt(m_socket, SOL_SOCKET, SO_SNDBUF, (const char*)&size, sizeof(size)) == SOCKET_ERROR) {
		log_socket_error("set_send_buffer");
		return false;
	}
	return true;
}

bool ix::socket::Socket::set_recv_buffer(int size)
{
	if (setsockopt(m_socket, SOL_SOCKET, SO_RCVBUF, (const char*)&size, sizeof(size)) == SOCKET_ERROR) {
		log_socket_error("set_recv_buffer");
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
		log_socket_error("set_linger");
		return false;
	}
	return true;
}

bool ix::socket::Socket::set_keepalive()
{
	int flag = 1;
	if (setsockopt(m_socket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&flag, sizeof(flag))) {
		log_socket_error("set_keepalive");
		return false;
	}
	return true;
}

bool ix::socket::Socket::set_reuseAddr()
{
	int flag = 1;
	if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, sizeof(flag))) {
		log_socket_error("set_reuseAddr");
		return false;
	}
	return true;
}



ix::socket::Socket::Socket(SOCKET socket) : m_ip(""), m_port(0), m_socket(socket)
{

}

Socket::~Socket()
{
	close();
}


void log_socket_error(const std::string& context)
{
	int err = WSAGetLastError();
	error("%s failed: error=%d, errmsg=%s",
		context.c_str(),
		err,
		ix::utility::Logger::Instance().PrintErrno(err).c_str());
}