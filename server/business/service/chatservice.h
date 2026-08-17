#pragma once

#include "../../../common/protocol/message.h"
class Session;
class ChatService
{
    public:
    static ChatService& instance();
    static void registerHandler();
    void PrivateChat(const Message& message,Session* session);
    void PrivateChatRead(  const Message& message,Session* session);
    void handlePrivateUnreadRequest(const Message& msg,const std::shared_ptr<Session>& session);
    private:
    ChatService() = default;
};