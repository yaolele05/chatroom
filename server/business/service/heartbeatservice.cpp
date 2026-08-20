#include "heartbeatservice.h"
#include "../../session/sessionmanager.h"
#include "../../session/usersession.h"

#include "../../database/connectionpool/redispool.h"
#include "../../database/redis/redisclient.h"
#include <nlohmann/json.hpp>
#include <string>
#include <iostream>
#include "../businessdispatcher/businessdispatcher.h"
using json=nlohmann::json;
HeartbeatService& HeartbeatService::instance()
{
   static HeartbeatService service;
   return service;
}

void HeartbeatService::registerHandler()
{
    BusinessDispatcher::instance().registerHandler(Messagetype::HeartBeat, [](const Message& msg,Session* session)
    {
     HeartbeatService::instance().heartbeat(msg, session);
    });
}
void HeartbeatService::heartbeat(const Message&,Session* session)
{
    if(session==nullptr)
    {
        return;
    }
    if(!session->authenticated())
    {
        return;
    }
    updateSession(session);
    updateRedis(session);
}
void HeartbeatService::updateSession(Session* session)
{
    auto userSession=dynamic_cast<UserSession*>(session);
    if(userSession==nullptr)
    {
        return;
    }

    userSession->updateHeartbeat();
}
void HeartbeatService::updateRedis(Session* session)
{
   auto redis=RedisPool::instance().getConnection();
   if(!redis)
   {
    return;
   }
   auto userSession=dynamic_cast<UserSession*>(session);

   if(userSession==nullptr)
   {
    return;
   }
    redis->refreshHeartbeat(userSession->userid());

}