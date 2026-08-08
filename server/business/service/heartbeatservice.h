#pragma once
#include"../../session/session.h"
#include <nlohmann/json.hpp>

#include "../../../common/protocol/message.h"
class HeartbeatService
{
    public:
    static HeartbeatService& instance();
    static void registerHandler();
    void heartbeat(const Message& message,Session* session);
    
         

    private:
    HeartbeatService()=default;
    void updateSession(Session*session);
    void updateRedis(Session* session);

};