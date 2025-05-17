#include "Session.h"
#include <iostream>
#include <nlohmann/json.hpp>
using namespace std;

namespace ix::m_boost::session
{
Session::Session(boost::asio::io_context& io_context,std::shared_ptr<boost::asio::ip::tcp::socket>socket,int id) : io(io_context) ,m_socket(socket),tmpBuf{}
{
    info->id = id;
}

void Session::update_info(const int id,const string& username,const string& password,const string& nickname)
{
    info->id = id;
    info->isOnline = true;
    info->username = username;
    info->password = password;
    info->nickname = nickname;
}

// void Session::recv()
// {
//     auto self = shared_from_this();
//     cout << "等待消息" << endl;
//     boost::asio::post(m_socket.get_executor(),[this]()
//     {
//         do_read();
//     });
// }
void Session::do_read()
{
    boost::system::error_code ec;
    auto self = shared_from_this();

    m_socket->async_read_some(boost::asio::buffer(tmpBuf),
        [this,self](const boost::system::error_code &ec,size_t length)
        {
            if (!ec)
            {
                //cout << "接收到数据长度: " << length << endl;
                recvBuffer.insert(recvBuffer.end(),tmpBuf,tmpBuf+length);
                post(io,[self,length]()
                {
                    self->process_data();
                    self->do_read();
                });
            }
            else
            {
                handle_error(ec);
            }
        });
}
void Session::send(shared_ptr<vector<char>> msg)
{
    auto self = shared_from_this();
    boost::asio::post(m_socket->get_executor(),[self,msg]()
    {
        bool isWriting;
        {
            isWriting = !self->sendQueue.empty();
            //cout << "send " << string(msg->begin(),msg->end()) << endl;
            self->sendQueue.push(msg);
        }
        if (!isWriting)
        {
            self->do_write();
        }
    });
}
void Session::do_write()
{
    shared_ptr<vector<char>> msg;
    auto self = shared_from_this();
    {
        lock_guard<mutex> lock(sendQueue_mtx);
        msg = sendQueue.front();
        sendQueue.pop();
    }

    m_socket->async_write_some(boost::asio::buffer(*msg),
        [self](const boost::system::error_code &ec,size_t length)
    {
        if (!ec)
        {
            if (!self->sendQueue.empty())
            {
                self->do_write();
            }
            else
            {
                //cout << "数据全部发送完毕" << endl;
            }
        }
        else
        {
            cout << self->info->username << " ";
            self->handle_error(ec);
        }
    });
}
void Session::handle_error(const boost::system::error_code &ec)
{
    if (ec == boost::asio::error::connection_aborted)
    {
        cout << "网络错误" << endl;
        m_socket->close();
    }
    else if (ec == boost::asio::error::eof)
    {
        cout << "对方优雅关闭了连接" << endl;
        disconnect();
    }
    else
    {
        cout << ec.message() << endl;
    }
}
//处理缓冲区
void Session::process_data()
{
    while (true)
    {
        auto header = trimAndGetHeader();
        if (!header)
        {
            break;
        }
        auto fragments = segments[header->MessageId].fragments;
        fragments.emplace(header->FragmentId,make_shared<vector<char>>(recvBuffer.begin(),recvBuffer.begin()+header->thisSize));
        recvBuffer.erase(recvBuffer.begin(),recvBuffer.begin() + header->thisSize);
        //如果包数量达到包头描述的大小
        if (fragments.size() == header->FragmentsCount)
        {
            //cout << "数据处理完毕，将交付给 data_handler" << endl;
            auto curPackets = fragments;
            segments.erase(header->MessageId);
            auto self = shared_from_this();
            uint32_t jsonSize = header->jsonSize;

            io.post([self,curPackets,jsonSize]()
            {
                //cout << "jsonSize : " << jsonSize << endl;
                auto fullMsg = *self->merge_fragments(curPackets);
                string data(fullMsg->begin(),fullMsg->begin()+jsonSize);
                fullMsg->erase(fullMsg->begin(),fullMsg->begin() + jsonSize);

                try
                {
                    //cout << data << endl;
                    nlohmann::json j = nlohmann::json::parse(data);
                    j["from_id"] = j.value("from_id",self->info->id);
                    self->data_handler(fullMsg,j);
                }
                catch (const exception& e)
                {
                    cout << e.what() << endl;
                }
            });
        }
    }
}

userInfo & Session::get_info()
{
    return *info;
}

void Session::handle_disconnect()
{
}
//切割和获取包头
optional<S2C_agreement> Session::trimAndGetHeader()
{
    int headerSize = sizeof(S2C_agreement);
    if (recvBuffer.size() < headerSize)
    {
        if (recvBuffer.size())
        {
            cout << "包头不完整" << endl;
        }
        return std::nullopt;
    }
    S2C_agreement s2c;

    memcpy(&s2c,recvBuffer.data(),headerSize);
    auto ntohl_agreement = handle_ntohl(s2c);
    if (recvBuffer.size() < ntohl_agreement.thisSize + headerSize)
    {
        cout << "包体不完整" << endl;
        return std::nullopt;
    }
    recvBuffer.erase(recvBuffer.begin(),recvBuffer.begin()+headerSize);
    return ntohl_agreement;
}

optional<shared_ptr<vector<char>>> Session::merge_fragments(FragmentMap fragments)
{
    int proofreadNum = 0;
    shared_ptr<vector<char>> fullMessage = make_shared<vector<char>>();
    for (auto fragment : fragments)
    {
        if (proofreadNum != fragment.first)
        {
            cout << "包不完整,退出合并" << endl;
            return std::nullopt;
        }
        fullMessage->insert(fullMessage->begin(),fragment.second->begin(),fragment.second->end());
        //cout << "merge: " << string(fullMessage->begin(),fullMessage->end()) << endl;
        proofreadNum++;
    }

    return fullMessage;
}


// void Session::distribute(uint32_t jsonSize, shared_ptr<vector<char>> data)
// {
//     nlohmann::json j = nlohmann::json::parse(data->begin(),data->begin()+jsonSize);
//     data->erase(data->begin(),data->begin()+jsonSize);
//     auto self = shared_from_this();
//
//     auto process = [self,j,data](const string& action)
//     {
//         if (action == "login")
//         {
//             self->handle_data(move(j),data);
//         }
//         else if (action == "chat")
//         {
//             self->handle_chat(move(j),data);
//         }
//     };
//     io.post([process,j]()
//     {
//         process(j["action"]);
//     });
// }
void Session::set_data_handler(DataHandler&& data_handler)
{
    this->data_handler = move(data_handler);
}

S2C_agreement Session::handle_ntohl(S2C_agreement agreement)
{
    agreement.jsonSize = ntohl(agreement.jsonSize);
    agreement.MessageId = ntohl(agreement.MessageId);
    agreement.FragmentsCount = ntohl(agreement.FragmentsCount);
    agreement.FragmentId = ntohl(agreement.FragmentId);
    agreement.thisSize = ntohl(agreement.thisSize);
    return agreement;
}

void Session::disconnect()
{
    auto self = shared_from_this();
    io.post([self]()
    {
        nlohmann::json data;
        data["action"] = "disconnect";
        data["id"] = self->info->id;
        data["username"] = self->info->username;
        try
        {
            data["from_id"] = data.value("from_id",self->info->id);
            self->data_handler({},data);
        }
        catch (const exception& e)
        {
            cout << e.what();
        }
    });
}

}