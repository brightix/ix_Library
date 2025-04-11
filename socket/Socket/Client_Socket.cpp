#include "Client_Socket.h"
#include "../../utility/JsonUtility/JsonUtility.h"
#include "../../utility/ThreadPool_singleton/ThreadPool_singleton.h"
#include "../Logger/Logger.h"
#include <iostream>
#include <thread>
#include <netinet/in.h>


using namespace ix::socket;
using namespace std;

extern ix::thread::ThreadPool& threadPool;

// 连接层
bool Client_Socket::login()
{
	string userName;
	string password;
	cout << "用户名： ";getline(cin,userName);
	cout << "密码： "; getline(cin,password);
	size_t userNameSize = userName.size();
	size_t passwordSize = password.size();
	vector<char> loginData = ix::utility::JsonUtility::PacketHeader<LOGIN>(userName, userNameSize, password, passwordSize);

	send_to("Server",loginData,{});
	wait_for_login();
	if (loginStatus == 1)
	{
		return true;
	}
	cout << "账号或密码错误" << endl;
	return false;
}

void Client_Socket::wait_for_login()
{
	uint64_t u;
	read(login_event_fd,&u,sizeof(u));
}

void Client_Socket::onOnline(const string& ip,int port)
{
	if (isConnected)
	{
		cout << "请勿反复操作连接服务器" << endl;
		return;
	}
	if (isConnected = connect(ip,port))
	{
		uint64_t u = 1;
		write(connect_event_fd,&u,sizeof(u));
		cout << "已连接至服务器" << endl;
	}
	else
	{
		cout <<"连接服务器 '失败'" << endl;
	}
}

void Client_Socket::onOffline()
{
	textQueue = std::queue<std::pair<std::string,std::shared_ptr<std::string>>>();

	recvBuffers.clear();

	user = UserInfo();
	loginStatus = 0;

	receiveDataStores = make_unique<std::unordered_map<uint32_t,std::pair<uint32_t,std::map<uint32_t,std::shared_ptr<std::vector<char>>>>>>();

	isConnected = false;
	uint64_t u = 1;
	write(connect_event_fd,&u,sizeof(u));

}

//收发数据层
void Client_Socket::send_to(string to,vector<char> packageHeader ,vector<char> data)
{
	//发送包头描述符
	uint32_t len = htonl(static_cast<uint32_t>(packageHeader.size()));
	send(reinterpret_cast<const char *>(&len),4);
	//发送包头
	send(packageHeader.data(),packageHeader.size());
	//发送数据
	send(data.data(),data.size());
}

void Client_Socket::receive_From_Client()
{
	while (!textQueue.empty())
	{
		auto cur = textQueue.front();
		textQueue.pop();
		cout << cur.first << *cur.second.get() << endl;
	}
}

bool ix::socket::Client_Socket::GetConnectStatus()
{
	return isConnected;
}

UserInfo & Client_Socket::GetUserInfo() { return user; }



bool Client_Socket::init()
{
	if (!Epoll_init())
	{
		error("Epoll_init");
		return false;
	}
	if (!epoll_control(m_socket,EPOLL_CTL_ADD,EPOLLIN | EPOLLET))
	{
		PRINT_ERRNO("epoll_ctl错误");
	}
	return true;
}

void Client_Socket::receive(int from_fd,vector<char>& recvBuffer)
{

	S2C_Agreement header ;
	size_t header_size = sizeof(header);
	char tmpBuf[1024]{};
	int len = recv(tmpBuf,sizeof(tmpBuf));
	if (len < 0)
	{
		if (errno == EINTR) {
			// 系统中断，可重试
			return;
		}
		if (errno == EPIPE || errno == ECONNRESET)
		{
			cout << "套接字挂了" << endl;
		}
		else {
			// 其他错误，基本可以认为连接挂了
			perror("recv failed");
		}
		cout << "连接断开" << endl;
		onOffline();
		return;
	}
	recvBuffer.insert(recvBuffer.end(),tmpBuf,tmpBuf + len);

	while (recvBuffer.size() >= header_size)
	{
		memcpy(&header,recvBuffer.data(),header_size);
		size_t totalPacketSize = header_size + header.residualByte;
		if (recvBuffer.size() < totalPacketSize)
		{
			return;
		}
		vector<char> body(recvBuffer.begin() + header_size,recvBuffer.begin() + totalPacketSize);
		handle_packet(header,body);
		recvBuffer.erase(recvBuffer.begin(),recvBuffer.begin() + totalPacketSize);
	}
}

void Client_Socket::receive_message()
{
	while (true)
	{
		while(!isConnected)
		{
			if (everythingDead)
			{
				return;
			}
			cout << "等待连接服务器连接" << endl;
			uint64_t u;
			read(connect_event_fd,&u,sizeof(u));
		}

		if (everythingDead)
		{
			return;
		}
		int num = wait();
		//预留多服务器eventfd

		for (size_t i = 0; i < num; i++)
		{
			if (evs[i].data.fd == stop_event_fd){
				uint64_t u;
				read(stop_event_fd, &u, sizeof(u));
				return;
			}
			receive(m_socket,recvBuffers[m_socket]);
		}
	}
}

void Client_Socket::handle_packet(S2C_Agreement s2c ,vector<char> package)
{
	if (receiveDataStores->find(s2c.Id) == receiveDataStores->end())
	{
		cout << "新任务" << endl;
		(*receiveDataStores)[s2c.Id].first = s2c.jsonSize;
	}
	auto& packet = (*receiveDataStores)[s2c.Id];
	packet.second[s2c.subPacketId] = make_shared<vector<char>>(package);
	if (packet.second.size() == s2c.totalPacketCount)
	{
		shared_ptr<vector<char>> tmp = merge_packets(packet.second);
		if (tmp != nullptr)
		{
			//packetBundles.push(tmp);
			cout << "数据校验 成功 ,即将被处理" << endl;

#ifdef mul_thread
			threadPool.submit([=]()
			{
				return handle_data((*receiveDataStores)[s2c.Id].first,tmp);
			});
#else
			handle_data((*receiveDataStores)[s2c.Id].first,tmp);
#endif
		}
		else
		{
			cout << "数据校验 失败 ,数据被清除" << endl;
		}
		//(*receiveDataStores).erase(s2c.Id);
	}
}

shared_ptr<vector<char>> Client_Socket::merge_packets(map<uint32_t,shared_ptr<vector<char>>> packets)
{
	uint32_t sequenceId = 0;
	shared_ptr<vector<char>> bound = make_shared<vector<char>>();
	for (auto packet : packets)
	{
		if (sequenceId == packet.first)
		{
			bound->insert(bound->end(),packet.second->begin(),packet.second->end());
			sequenceId ++;
		}
		else
		{
			return nullptr;
		}
	}
	return bound;
}

void Client_Socket::handle_data(uint32_t jsonSize,shared_ptr<vector<char>> data)
{
	nlohmann::json header;
#ifdef DEBUG_MODE
	cout << string(data->begin(),data->begin()+jsonSize) << endl;
#endif
	header = ix::utility::JsonUtility::GetJson(jsonSize,data.get());
	if (header["type"] == "login")
	{
		if (header["loginStatus"] == "success")
		{
			user.userName = header["userName"];
			loginStatus = 1;
		}
		else
		{
			loginStatus = -1;
		}
		uint64_t u = 1;
		write(login_event_fd,&u,sizeof(u));
	}
	else if (header["type"] == "text")
	{
		string message = string(data->begin()+jsonSize,data->end());
		cout << header["from"]<< ": " + message << endl;
	}
}

Client_Socket::Client_Socket(const std::string& ip, int port) : Socket(),Epoll()
{
	set_recv_buffer(10 * 1024);
	set_send_buffer(10 * 1024);
	set_linger(true, 0);
	//set_keepalive();
	//set_reuseAddr();
	onOnline(ip,port);
	receiveDataStores = make_unique<std::unordered_map<uint32_t,std::pair<uint32_t,std::map<uint32_t,std::shared_ptr<std::vector<char>>>>>>();
}

Client_Socket::Client_Socket(int file_describe):Socket(file_describe) {
	isConnected = true;
}

void Client_Socket::shutdown()
{
	onOffline();
	uint64_t u = 1;
	everythingDead = true;
	write(stop_event_fd,&u,sizeof(u));
}