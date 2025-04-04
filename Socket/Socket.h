#pragma once

#include <string>
#include <ws2tcpip.h>

namespace ix
{
	namespace socket 
	{
		class Socket
		{
		public:
			Socket();
			Socket(SOCKET socket);
			~Socket();
			bool bind(const std::string& ip,const int port);
			bool listen(int backlog);
			bool connect(const std::string& ip,const int port);
			SOCKET accept();

			int send(const char* buf,int len);
			int recv(char* buf, int len);
			void close();

			
			bool set_non_blocking();
			bool set_send_buffer(int size);
			bool set_recv_buffer(int size);
			bool set_linger(bool active,int seconds);
			bool set_keepalive();
			bool set_reuseAddr();
		protected:
			SOCKET m_socket;
			std::string m_ip;
			int m_port;

		};
	}
}

