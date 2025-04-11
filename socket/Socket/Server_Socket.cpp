#include "Server_Socket.h"
#include "../../utility/ThreadPool_singleton/ThreadPool_singleton.h"
#include "../../utility/JsonUtility/JsonUtility.h"
#include "../Epoll/Epoll.h"

#include <Client_Socket.h>
#include <format>
#include <iostream>
#include <Logger.h>
#include <netinet/in.h>
#include <sys/eventfd.h>
using namespace ix::socket;
using namespace std;
using json = nlohmann::json;

extern ix::thread::ThreadPool& threadPool;



void Server_Socket::recv_to_buffer(int fd)
{
	{
		lock_guard<mutex> lock(clients_mutex);
		auto it = clients.find(fd);
		if (it == clients.end()) {
			cout << "客户端 [" << fd << "] 已关闭，跳过处理" << endl;
			return;
		}
	}
	char buf[1024]{};
	while (true)
	{
		int len = clients[fd]->recv(buf,1024);
		if (len == -1)
		{
			if (errno == EAGAIN)
			{
				// 缓冲区没有数据
				break;
			}
			if (errno == ECONNRESET || errno == EPIPE) {
				error(ix::utility::Logger::Instance().PrintErrno("客户端连接被关闭 (Connection reset by peer)").c_str());
				return;
			}
			else
			{
				error(ix::utility::Logger::Instance().PrintErrno("文件处理失败").c_str());
				perror("文件处理失败（recv）");
				return;
			}
		}
		if (len == 0)
		{
			return;
		}
		recvBuffer[fd].insert(recvBuffer[fd].end(),buf,buf+len);
	}
}

vector<char> Server_Socket::handle_data(int fd,size_t totalSize)
{
	{
		lock_guard<mutex> lock(clients_mutex);
		auto it = clients.find(fd);
		if (it == clients.end()) {
			cout << "客户端 [" << fd << "] 已关闭，跳过处理" << endl;
			return {};
		}
	}

	if (recvBuffer[fd].size() < totalSize)
	{
		cout << "handle data的大小超过了缓存区大小" << endl;
		cout << "因该是还没有发送完毕" << endl;
		return {};
	}
	vector<char> data(recvBuffer[fd].begin(),recvBuffer[fd].begin()+totalSize);
	recvBuffer[fd].erase(recvBuffer[fd].begin(),recvBuffer[fd].begin()+totalSize);
	return data;
}




void Server_Socket::queueSend_new(int fd,vector<char> header,vector<char> data)
{
	S2C_Agreement s2c{};
	size_t totalLen = header.size() + data.size();
	s2c.jsonSize = header.size();

	header.insert(header.end(),data.begin(),data.end());

#ifdef DEBUG_MODE
	cout << string(header.begin(),header.end()) << endl;
	cout << "jsonSize  " << s2c.jsonSize << endl;
#endif

	s2c.Id = PACKET_ID++;//每次发大包都需要递增包ID
	s2c.totalPacketCount = (totalLen+MaxPacketSize) / MaxPacketSize;

	s2c.residualByte = totalLen;
	size_t len = 0;
	while (len < totalLen)
	{
		if (s2c.residualByte < 0)
		{
			cout << "？？？有问题，为什么residualByte会小于0" << endl;
		}
		size_t curLen = min(MaxPacketSize - 20,totalLen - len);//获取当前应该发的包体大小
		vector<char> packet;


		s2c.residualByte = curLen;// 包体数据大小
		packet.insert(packet.end(),reinterpret_cast<char*>(&s2c), reinterpret_cast<char*>(&s2c) + sizeof(s2c));
		packet.insert(packet.end(),header.data()+len,header.data() + len + curLen);

		send_to_client(fd, packet);
		len += curLen;
		s2c.subPacketId++;
	}
}

void Server_Socket::run()
{
	bool running = true;
	while (running)
	{
		const int num = wait();

		for (size_t i = 0; i < num; i++)
		{

			int currentFd = evs[i].data.fd;
			if (currentFd == stop_event_fd)
			{
				running = false;
				continue;
			}
#ifdef mul_thread
			threadPool.submit(
			[&,currentFd]()
			{
				handle_epoll_event(currentFd);
			});
#else
			handle_epoll_event(currentFd);
#endif
		}
	}
}

int Server_Socket::get_user_fd(string& userName)
{
	if (local_clients.find(userName) != local_clients.end())
	{
		return local_clients[userName].fd;
	}
	return -1;
}

int Server_Socket::has_user_in_local_table(std::string &userName)
{
	return local_clients.find(userName) != local_clients.end();
}

bool Server_Socket::is_user_online(string& userName)
{
	return local_clients.find(userName) != local_clients.end() && local_clients[userName].isOnline;
}

bool Server_Socket::check_user_password(std::string &userName,string& password)
{
	return local_clients.find(userName) != local_clients.end() && local_clients[userName].password == password;
}

void Server_Socket::loadLocalUser()
{
	local_clients.emplace("ix",ClientInfo({"ix","123"}));
	local_clients.emplace("dreamy",ClientInfo({"dreamy","456"}));
}

void Server_Socket::print_all_client()
{

	std::lock_guard<std::mutex> lock(clients_mutex);
	{
		if (clients.empty())
		{
			cout << "当前没有用户连接" << endl;
			return;
		}
		for (auto& c : clients)
		{
			cout << "[" << c.first << "]连接正常" << endl;
		}
	}

}

bool Server_Socket::checkClientRegistrationStatus(int cfd)
{
	return clients.find(cfd) != clients.end();
}






void Server_Socket::send_to_client(int cfd,std::vector<char> data)
{
	if (clients.find(cfd) == clients.end())
	{
		return;
	}
	else if (clients[cfd]->send(data.data(),data.size()) == -1)
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
#ifdef DEBUG_MODE
	cout << "数据发送成功" << endl;
#endif
}

void Server_Socket::modify_client_event(int cfd)
{
	epoll_control(cfd,EPOLL_CTL_MOD,EPOLLIN | EPOLLET | EPOLLONESHOT);
}


void Server_Socket::close_client(int cfd)
{
	epoll_control(cfd, EPOLL_CTL_DEL, 0); // 确保移除事件监听
	{
		std::lock_guard<std::mutex> lock(clients_mutex);
		clients.erase(cfd); // 删除客户端
	}
	cout << fd_name[cfd] << "[" << cfd << "]已经断开并关闭" << endl;
}

int Server_Socket::register_client() {
	int cfd = accept();
	{
		lock_guard<mutex> lock(clients_mutex);
		if (clients.find(cfd) != clients.end()) {
			close_client(cfd);
		}
		clients.emplace(cfd,make_unique<Client_Socket>(cfd));
	}
	clients[cfd]->set_non_blocking();
	epoll_control(cfd,EPOLL_CTL_ADD,EPOLLIN | EPOLLET | EPOLLONESHOT);
	return cfd;
}

void Server_Socket::loadUser(int fd , string& name)
{
	name_fd[name] = fd;
	fd_name[fd] = name;
}

bool Server_Socket::check_userName(std::string userName)
{
	return name_fd.find(userName) == name_fd.end();
}

void Server_Socket::shut_down()
{
	uint64_t u = 1;
	write(curb_fd,&u,sizeof(u));
	write(stop_event_fd,&u,sizeof(u));
}

//功能

//转发讯息
void Server_Socket::handle_epoll_event(int requestFd)
{

	if (requestFd == m_socket)
	{
		int cfd = register_client();
		clientsData.insert({cfd,ClientDataStore()});
	}
	else
	{
		recv_to_buffer(requestFd);
		while (recvBuffer[requestFd].size() >= clientsData[requestFd].nextSize)
		{
			if (!checkClientRegistrationStatus(requestFd))
			{
				cout << "事件请求者已不存在" << endl;
				return;
			}
			auto& client = clientsData[requestFd];
			vector<char> data = handle_data(requestFd,clientsData[requestFd].nextSize);
			if (client.currentState == WAIT_FOR_HEADER_DESCRIPTOR)
			{
				// 获取下一次发来包头的长度
				if (data.empty())
				{
					close_client(requestFd);
					break;
				}
				else
				{
					uint32_t netlen;
					memcpy(&netlen,data.data(),sizeof(uint32_t));
					client.nextSize = ntohl(netlen);
					client.currentState = WAIT_FOR_HEADER;
				}
			}
			else if (client.currentState == WAIT_FOR_HEADER)
			{
				if (data.empty())
				{
					//close_client(requestFd);
					break;
				}
				else
				{
					client.header = ix::utility::JsonUtility::GetJson(&data);
					client.nextSize = client.header["size"];
					client.currentState = WAIT_FOR_BODY;
				}
			}
			else
			{
				int used = client.header["UsedTo"].get<int>();
				if (data.empty() && used)
				{
					close_client(requestFd);
				}
				else
				{
					UsedTo usedTo = static_cast<UsedTo>(client.header["UsedTo"].get<int>());


					json j;
					switch (usedTo)
					{
						case LOGIN:
						{
							string userName = client.header["userName"];
							//检查服务器是否有该用户数据
							string password = client.header["password"];

							j["type"] = "login";
							j["loginStatus"] = check_user_password(userName,password)? "success" : "failed";
							j["userName"] = userName;

							loadUser(requestFd,userName);
							local_clients[userName].fd = requestFd;
							if (has_user_in_local_table(userName) != -1)
							{
								//登陆成功
								local_clients[userName].isOnline = true;
								cout  << userName << "[" << requestFd << "]" << "已上线" << endl;
							}
							else
							{
								//登陆失败
								j["loginStatus"] = "failed";
							}
							string header = j.dump();
							queueSend_new(requestFd,vector<char>(header.begin(),header.end()),data);
							break;
						}
						case CHAT:
						{
							//收件人 fd
							string to = client.header["to"];
							int fd = has_user_in_local_table(to);
							if (fd == -1)
							{
								string ret = "查无此人";
								send_to_client(requestFd,vector(ret.begin(),ret.end()));
							}
							else
							{
								if (is_user_online(to))
								{
									j["type"] = "text";
									j["UsedTo"] = usedTo;
									j["from"] = fd_name[requestFd];
									j["to"] = to;
									j["fileName"] = "text";
									j["size"] = data.size();
									string header = j.dump();
									queueSend_new(name_fd[to],vector<char>(header.begin(),header.end()),data);
									cout << "发送给" + to << endl;
								}
							}
							break;
						}
					}
					client.nextSize = 4;
					client.currentState = WAIT_FOR_HEADER_DESCRIPTOR;
					}
				}
			}
		}

	// 重新标记fd为可执行状态
	{
		std::lock_guard<std::mutex> lock(clients_mutex);
		if (clients.find(requestFd) != clients.end()) {
			modify_client_event(requestFd);
		}
	}
}

Server_Socket::Server_Socket(const std::string& ip, int port) : Socket() , Epoll()
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
	Epoll_init();
	while (!epoll_control(m_socket,EPOLL_CTL_ADD,EPOLLIN | EPOLLET))
	{
		PRINT_ERRNO("epoll_ctl错误，正在重试");
		error("epoll_ctl错误");
		this_thread::sleep_for(chrono::seconds(1));
	}

	//加载本地用户
	loadLocalUser();
}

Server_Socket::~Server_Socket()
{
	{
		lock_guard<mutex> lock(clients_mutex);
		clients.clear();
	}
	clientsData.clear();
	clientSendTasks.clear();
}