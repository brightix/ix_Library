#include "Server_Socket.h"
#include "../../utility/ThreadPool/ThreadPool.h"

#include <Client_Socket.h>
#include <format>
#include <iostream>
#include <Logger.h>
using namespace ix::socket;
using namespace std;
static long long runTime = 0;
Server_Socket::Server_Socket(const std::string& ip, int port) : Socket()
{
	set_recv_buffer(10 * 1024);
	set_send_buffer(10 * 1024);
	set_linger(true, 0);
	set_keepalive();
	set_reuseAddr();
	bind(ip,port);
	listen(1024);

	//epoll
	size = std::size(evs);
}

Server_Socket::~Server_Socket()
{
	clients.clear();
}

/* epoll */
void Server_Socket::init_epoll() {
	epfd = epoll_create(1);
}

int Server_Socket::Get_epfd() {
	return epfd;
}

struct epoll_event* Server_Socket::Get_evs() {
	return evs;
}

bool Server_Socket::epoll_control(int fd,int op,struct epoll_event&& ev)
{
	return epoll_ctl(epfd,op,fd,&ev) != -1;
}

int Server_Socket::wait() {
	int ret = epoll_wait(epfd,evs,size,-1);
	return ret;
}

void Server_Socket::register_client() {
	int cfd = accept();
	if (clients.find(cfd) != clients.end()) {
		close_client(cfd);
	}
	clients.emplace(cfd,make_unique<Client_Socket>(cfd));
	clients[cfd]->set_non_blocking();
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	ev.data.fd = cfd;
	epoll_control(cfd,EPOLL_CTL_ADD,move(ev));
	printNow = true;
	print_condition.notify_one();
	//cout << "用户[" << cfd << "]连接至服务器" << endl;
}

string Server_Socket::handle_data(int fd) {
	if (clients.find(fd) == clients.end()) {
		cout << "客户端 [" << fd << "] 已关闭，跳过处理" << endl;
		return "";
	}
	string data;
	char buf[8]{};
	while (true)
	{
		int len = clients[fd]->recv(buf,sizeof(buf)-1);
		if (len == -1)
		{
			if (errno == EAGAIN)
			{
				// 缓冲区没有数据
				break;
			}
			if (errno == ECONNRESET || errno == EPIPE) {
				error(ix::utility::Logger::Instance().PrintErrno("客户端连接被关闭 (Connection reset by peer)").c_str());
				close_client(fd);
				return "";
			}
			else
			{
				error(ix::utility::Logger::Instance().PrintErrno("文件处理失败").c_str());
				perror("文件处理失败（recv）");
				break;
			}
		}
		else
		{
			buf[len] = '\0';
			data += buf;
		}
	}
	runTime++;
	return data;
}

void Server_Socket::print_all_client()
{
	std::lock_guard<std::mutex> lock(print_mutex);
	{
		for (auto& c : clients)
		{
			cout << "[" << c.first << "]连接正常" << endl;
		}
	}

}

void Server_Socket::print_clients_periodically()
{
	while (true)
	{
		unique_lock<mutex> lock(print_mutex);
		print_condition.wait(lock,[this](){ return printNow; });
		print_all_client();
		printNow = !printNow;
	}
}

void Server_Socket::send_to_client(int cfd,string data)
{

	if (clients[cfd]->send(data.c_str(),data.size()) == -1)
	{
		if (errno == EPIPE || errno == ECONNRESET)
		{
			error(ix::utility::Logger::Instance().PrintErrno("send").c_str());
			close_client(cfd);
			return;
		}
		else
		{
			perror("send error");
			error(ix::utility::Logger::Instance().PrintErrno("send error").c_str());
			return;  // 或者抛出异常
		}
	}
	//cout << "data ->Client[" << cfd << "] success..." << endl;
}

void Server_Socket::modify_client_event(int fd)
{
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
	ev.data.fd = fd;
	epoll_control(fd,EPOLL_CTL_MOD,move(ev));
}


void Server_Socket::close_client(int fd)
{
	struct epoll_event ev;
	ev.events = 0; // 没有监听任何事件
	ev.data.fd = fd;
	epoll_control(fd, EPOLL_CTL_DEL, std::move(ev)); // 确保移除事件监听
	{
		std::lock_guard<std::mutex> lock(clients_mutex);
		clients.erase(fd); // 删除客户端
	}
	cout << "客户端[" << fd << "]已经断开并关闭" << endl;
	cout << runTime << endl;
}
