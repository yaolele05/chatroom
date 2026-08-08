#include "chatserver.h"

#include "business/businessdispatcher/businessdispatcher.h"

#include "business/service/loginservice.h"
#include "business/service/friendservice.h"
#include "business/service/groupservice.h"
#include "business/service/chatservice.h"
#include "business/service/heartbeatservice.h"
#include "business/service/fileservice.h"
#include "chatserver.h"
#include "business/service/loginservice.h"
#include "business/service/friendservice.h"
#include "business/service/groupservice.h"
#include "business/service/chatservice.h"
#include "business/service/fileservice.h"
#include "business/service/heartbeatservice.h"
#include "business/service/offlineservice.h"
#include "database/connectionpool/mysqlpool.h"
#include "database/connectionpool/redispool.h"

#include <iostream>

int main()
{
  
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
    HeartbeatService::instance().registerHandler();
    // OfflineService::instance().registerHandler();
    EventLoop loop;
    InetAddress addr(8888);
    Chatserver server(&loop, addr);
    server.start();
    std::cout << "ChatServer started." << std::endl;
    loop.loop();

    return 0;
}