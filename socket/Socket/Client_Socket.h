#pragma once
#include <atomic>
#include <queue>
#include <unordered_set>
#include <vector>
#include <sys/eventfd.h>

#include "../Epoll/Epoll.h"

#include "Socket.h"
#include "../../utility/JsonUtility/JsonUtility.h"




struct UserInfo
{
    std::string userName;
};

namespace ix {
    namespace socket {
    class Client_Socket :
    public Socket , public Epoll
    {

        //序列号和数据内容
        //                                <数据序列号     ,     <json大小          <分包id                    数据流 > > >
        std::unique_ptr<std::unordered_map<uint32_t,std::pair<uint32_t,std::map<uint32_t,std::shared_ptr<std::vector<char>>>>>> receiveDataStores;

        //大包队列
        std::queue<std::shared_ptr<std::vector<char>>> packetBundles;

        //待接收消息队列
        std::queue<std::pair<std::string,std::shared_ptr<std::string>>> textQueue;
        //std::atomic<std::unordered_set<int>> isConnected;
        std::atomic_bool isConnected;
        std::unordered_map<int,Socket*> linkedList;
        std::unordered_map<int,std::vector<char>> recvBuffers;
    public:
        Client_Socket() = delete;
        Client_Socket(const std::string& ip,int port);
        explicit Client_Socket(int file_describe);

        void shutdown();

        bool init();

        void run();

        bool login();

        //重新登录
        void onOnline(const std::string &ip, int port);
        void onOffline();

        void send_to(std::string to, std::vector<char> packageHeader,std::vector<char> data);

        void receive_From_Client();

        void receive_message();
        bool GetConnectStatus();

        UserInfo& GetUserInfo();
        ~Client_Socket() = default;

        int login_event_fd = eventfd(0,EFD_CLOEXEC);
        int connect_event_fd = eventfd(0,0);
    private:


        S2C_Agreement handle_packageHeader(char *buf);

        void handle_packet(S2C_Agreement s2c ,std::vector<char> package);

        std::shared_ptr<std::vector<char>> merge_packets(std::map<uint32_t,std::shared_ptr<std::vector<char>>> packets);

        void handle_data(uint32_t jsonSize, std::shared_ptr<std::vector<char>> data);

        void receive(int from_fd, std::vector<char> &recvBuffer);

        void wait_for_login();




        std::atomic<bool> everythingDead = false;
        std::atomic<int> loginStatus = 0;
        UserInfo user;
    };
}
}


