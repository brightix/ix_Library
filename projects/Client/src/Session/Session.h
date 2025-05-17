#pragma once
#include <boost/asio.hpp>
#include <map>
#include <queue>
#include <nlohmann/adl_serializer.hpp>

#pragma pack(1)
struct S2C_agreement
{
    uint32_t MessageId;
    uint32_t FragmentId;
    uint32_t thisSize;
    uint32_t FragmentsCount;
    uint32_t jsonSize;
};
#pragma pack()

namespace ix::m_boost::session {

using DataHandler = std::function<void(std::shared_ptr<std::vector<char>>,nlohmann::json)>;
using FragmentMap = std::map<int, std::shared_ptr<std::vector<char>>>;

struct MessageBuffer
{
    //size_t remainingSize;
    FragmentMap fragments;
};

struct userInfo
{
    int id;
    std::string username;
    std::string password;
    std::optional<std::string> nickname;
    bool isOnline;
};

class Session :
    public std::enable_shared_from_this<Session>
{

    boost::asio::io_context& io;



    std::optional<userInfo> info;
    char  tmpBuf[4096]; //临时接收读取
    std::vector<char> recvBuffer;    //输入缓冲区

    std::queue<std::shared_ptr<std::vector<char>>> sendQueue;    //输出缓冲区
    std::mutex sendQueue_mtx;
    std::unordered_map<int, MessageBuffer> segments;   //碎片数据
    std::mutex segments_mtx;
    std::queue<std::shared_ptr<std::vector<char>>> data;   //完整数据队列
    std::mutex data_mtx;
public:
    std::shared_ptr<boost::asio::ip::tcp::socket> m_socket;
    virtual ~Session() = default;

    Session(boost::asio::io_context& io_context,std::shared_ptr<boost::asio::ip::tcp::socket> socket,int id);
//注册登陆信息
    void update_info(int id, const std::string &username, const std::string &password, const std::string &nickname);

//接收
    void recv();
    void do_read();

//发送
    void send(std::shared_ptr<std::vector<char>> msg);
    void do_write();

//处理
    void process_data();

//获取信息
    userInfo& get_info();
    std::optional<std::shared_ptr<std::vector<char>>> merge_fragments(FragmentMap fragments);

    //void distribute(uint32_t jsonSize, std::shared_ptr<std::vector<char>> data);

    void set_data_handler(DataHandler&& data_handler);

    S2C_agreement handle_ntohl(S2C_agreement agreement);

    void disconnect();

    //virtual void handle_login(nlohmann::json jsonSize, std::shared_ptr<std::vector<char>> data) = 0;
    //virtual void handle_chat(nlohmann::json jsonSize, std::shared_ptr<std::vector<char>> data) = 0;
private:


    void handle_error(const boost::system::error_code &ec);


    void handle_disconnect();

    std::optional<S2C_agreement> trimAndGetHeader();
    DataHandler data_handler;
};

}
