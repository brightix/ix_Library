//
// Created by ix on 25-6-13.
//
#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <map>
#include <queue>
#include <boost/asio.hpp>
#include "../package/package.pb.h"
using namespace std;
class UdpServer : enable_shared_from_this<UdpServer> {
    using FragmentMap = map<int, std::string>;
    string ip;
    string port;
    boost::asio::io_context io;
    boost::asio::ip::udp::socket m_socket;
    boost::asio::streambuf buffer;
    //vector<char> buffer;
    char buf[4096];
    queue<vector<char>> recv_buffer;



    queue<std::shared_ptr<std::vector<char>>> sendQueue;    //输出缓冲区
    std::mutex sendQueue_mtx;
    std::unordered_map<int, FragmentMap> segments;   //碎片数据
    std::mutex segments_mtx;
    std::queue<std::shared_ptr<std::vector<char>>> data;   //完整数据队列
    std::mutex data_mtx;


    //udp
    boost::asio::ip::udp::endpoint remote_endpoint;

    //回调函数
    function<void(string)> dispatcher;
public:
    UdpServer();

    UdpServer(string ip, string port);

    void read();

    void check_buffer();

    optional<string> merge_fragments(FragmentMap fragments);

    void send(const std::vector<char> &data, boost::asio::ip::udp::endpoint &endpoint);

    ~UdpServer();

    void handle_error(const boost::system::error_code &ec);

    //设置回调函数
    void set_dispatcher(function<void(string)> func);
};



#endif //TCPSERVER_H
