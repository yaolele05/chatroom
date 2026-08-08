#pragma once

#include "../../../common/protocol/message.h"
#include "../../session/session.h"
class GroupService
{
    public:
    
    static GroupService& instance();
    static void rigisterHandler();
    void createGroup(const Message& msg, Session* se);
    void joinGroup(const Message& msg,Session* se);
    void leaveGroup(const Message& msg,Session* se);
    void groupChat(const Message& msg,Session* se);
    void groupList(const Message& msg,  Session*se);
    private:
    GroupService()=default;
};