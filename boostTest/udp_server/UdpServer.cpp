//
// Created by ix on 25-6-13.
//
#include "UdpServer.h"

#include <iostream>
using namespace std;
using namespace boost::asio;
UdpServer::UdpServer() : ip({}), port({}), m_socket({}), buf{}
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
}

UdpServer::UdpServer(string ip,string port) : ip(ip), port(port), m_socket({}), buf{}
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    ip::udp::resolver resolver(io);
    auto endpoints = resolver.resolve(ip::udp::v4(), ip, port);
    m_socket.open(ip::udp::v4());
    m_socket.bind(*endpoints.begin());
    read();
}

UdpServer::~UdpServer()
{
    google::protobuf::ShutdownProtobufLibrary();
}

void UdpServer::read()
{
    m_socket.async_receive_from(boost::asio::buffer(buf),remote_endpoint,
        [self = shared_from_this()](const boost::system::error_code &ec,size_t length)
        {
            if (!ec)
            {
                self->recv_buffer.emplace(self->buf,self->buf + length);
                self->io.post([self]()
                {
                    self->check_buffer();
                    self->read();
                });
            }
            else
            {
                self->handle_error(ec);
            }
        });
}

void UdpServer::check_buffer()
{
    Wrapper wrapper;
    auto& b = recv_buffer.front();
    wrapper.ParseFromArray(b.data(),b.size());
    auto& fragments = segments[wrapper.id()];
    fragments.emplace(wrapper.segment_id(),wrapper.data());
    recv_buffer.pop();

    //如果包数量达到包头描述的大小
    if (fragments.size() == wrapper.segment_cnt())
    {
        //cout << "数据处理完毕，将交付给 data_handler" << endl;
        auto curPackets = std::move(fragments);
        segments.erase(wrapper.id());
        io.post([self = shared_from_this(),curPackets]()
        {
            if (optional<string> data = self->merge_fragments(curPackets))
            {
                self->io.post([self,data]
                {
                    //使用回调处理数据，和udp无关
                    self->dispatcher(*data);
                });
            }
        });
    }
}


optional<string> UdpServer::merge_fragments(FragmentMap fragments)
{
    int proofreadNum = 0;
    string fullMessage;
    for (auto& fragment : fragments | ranges::range)
    {
        if (proofreadNum != fragment.first)
        {
            cout << "包不完整,退出合并,丢包" << endl;
            return std::nullopt;
        }
        fullMessage += move(fragment);
        //cout << "merge: " << string(fullMessage->begin(),fullMessage->end()) << endl;
        proofreadNum++;
    }
    cout << "合并成功" << endl;
    return fullMessage;
}


void UdpServer::send(const std::vector<char>& data,boost::asio::ip::udp::endpoint& endpoint)
{

    m_socket.async_send_to(boost::asio::buffer(data),endpoint,
        [self = shared_from_this()](boost::system::error_code ec, std::size_t)
        {
            if (ec)
            {
                self->handle_error(ec);
            }
        });
}

void UdpServer::handle_error(const boost::system::error_code &ec)
{
    if (ec == boost::asio::error::connection_aborted)
    {
        cout << "网络错误" << endl;
        m_socket.close();
    }
    else if (ec == boost::asio::error::eof)
    {
        cout << "对方优雅关闭了连接" << endl;
        //disconnect();
    }
    else
    {
        cout << ec.message() << endl;
    }
}

void UdpServer::set_dispatcher(function<void(string)> func)
{
    if (func)
    {
        dispatcher = func;
    }
    else
    {
        cerr << "设置回调函数失败: set_dispatcher" << endl;
    }
}


