#include "JsonUtility.h"

#include <iostream>
#include <netinet/in.h>
//#define DEBUG_MODE

using json = nlohmann::json;
using namespace std;
namespace ix {
    namespace utility {
        namespace JsonUtility {

            nlohmann::json GetJson(uint32_t json_size,vector<char>* stream)
            {
#ifdef DEBUG_MODE
                cout << string(stream->begin(),stream->begin()+json_size) << endl;
#endif
                json j;
                try {
                    j = json::parse(stream->data(),stream->data()+json_size);
                    // 继续逻辑...
                } catch (const std::exception& e) {
                    std::cout << "解析 JSON 异常: " << e.what() << std::endl;
                    return {};
                }
                return j;
            }
            nlohmann::json GetJson(vector<char>* stream)
            {
#ifdef DEBUG_MODE
                cout << string(stream->begin(),stream->end()) << endl;
#endif
                json j;
                try {
                    j = json::parse(stream->data(),stream->data()+stream->size());
                    // 继续逻辑...
                } catch (const std::exception& e) {
                    std::cout << "解析 JSON 异常: " << e.what() << std::endl;
                    return {};
                }
                return j;
            }
            uint32_t PacketHeader_descriptor(size_t size)
            {
                return htonl(static_cast<uint32_t>(size));
            }

            std::vector<char> Chat(UsedTo usedTo,std::string from,std::string type, std::string to, std::string fileName, size_t size)
            {
                json header;
                header["UsedTo"] = usedTo;
                header["from"] = from;
                header["type"] = type;
                header["to"] = to;
                header["fileName"] = fileName;
                header["size"] = size;
                string t = header.dump();
                return vector<char>(t.begin(),t.end());
            }
            std::vector<char> Login(UsedTo usedTo,std::string userName, size_t name_size, std::string password,size_t password_size)
            {
                json header;
                header["UsedTo"] = usedTo;
                header["userName"] = userName;
                header["name_size"] = name_size;
                header["password"] = password;
                header["password_size"] = password_size;
                header["size"] = 0;
                string t = header.dump();
                return vector<char>(t.begin(),t.end());
            }
        } // namespace JsonUtility
    } // namespace utility
} // namespace ix
