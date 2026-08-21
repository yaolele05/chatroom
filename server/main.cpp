#include "chatserver.h"

#include "business/businessdispatcher/businessdispatcher.h"
#include "business/service/loginservice.h"
#include "business/service/friendservice.h"
#include "business/service/groupservice.h"
#include "business/service/chatservice.h"
#include "business/service/heartbeatservice.h"
#include "business/service/fileservice.h"
#include "business/service/offlineservice.h"
#include "business/service/historyservice.h"

#include "database/connectionpool/mysqlpool.h"
#include "database/connectionpool/redispool.h"

#include <iostream>
#include <string>
#include <curl/curl.h>

int main(int argc, char* argv[])
{
    std::string serverIp = "0.0.0.0";
    uint16_t serverPort = 8888;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--ip")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Error: --ip requires an argument\n";
                return -1;
            }

            serverIp = argv[++i];
        }
        else if (arg == "--port")
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Error: --port requires an argument\n";
                return -1;
            }

            try
            {
                int port = std::stoi(argv[++i]);

                if (port < 1 || port > 65535)
                {
                    std::cerr << "Error: invalid port\n";
                    return -1;
                }

                serverPort = static_cast<uint16_t>(port);
            }
            catch (const std::exception&)
            {
                std::cerr << "Error: invalid port\n";
                return -1;
            }
        }
        else
        {
            std::cerr << "Unknown argument: " << arg << '\n';
            return -1;
        }
    }

    std::cout << "Server address: " << serverIp << ":" << serverPort << std::endl;

    curl_global_init(CURL_GLOBAL_DEFAULT);
     if (!MysqlPool::instance().init("127.0.0.1",3306,"chatserver","123456","chatroom",8))
    {
        std::cerr << "MysqlPool init failed" << std::endl;
        return -1;
    }
    std::cout << "MysqlPool init success" << std::endl;
    if (!RedisPool::instance().init("127.0.0.1",6379,4))
    {
        std::cerr << "RedisPool init failed" << std::endl;
        return -1;
    }
    std::cout << "RedisPool init success" << std::endl;

    LoginService::instance().registerHandle();
    FriendService::instance().registerHandler();
    GroupService::instance().rigisterHandler();
    ChatService::instance().registerHandler();
    FileService::instance().registerHandler();
    HistoryService::instance().registerHandler();

    EventLoop loop;
    InetAddress addr(serverPort, serverIp);

    Chatserver server(&loop, addr);
    server.setThreadNum(3);
    server.start();

    std::cout << "ChatServer started at "<< serverIp << ":" << serverPort << std::endl;
  
    loop.loop();

    curl_global_cleanup();

    return 0;
}