#pragma once

#include <string>
#include <sys/epoll.h>
namespace ix
{
namespace socket
{

enum IOMultiplexing {
	Select,
	Poll,
	EPOLL
};

class Socket
{
public:
	Socket();
	explicit Socket(int socket);
	virtual ~Socket();

	/* 绑定服务器端口 */
	bool bind(const std::string& ip,int port);
	/* 设置最大监听等待队列 */
	bool listen(int backlog);
	/* 连接至指定端口 */
	bool connect(const std::string& ip,int port);
	/* 等待接听 */
	int accept();
	int send(const char* buf,size_t len);
	ssize_t recv(char* buf, size_t len);
	void close();

	/* Get */
	int Get_fd();

	/* Set */
	bool set_non_blocking();
	bool set_send_buffer(int size);
	bool set_recv_buffer(int size);
	bool set_linger(bool active,int seconds);
	bool set_keepalive();
	bool set_reuseAddr();
protected:



	int m_socket;
	std::string m_ip;
	int m_port;


};


}
}

