#pragma once

#include "../../protocol/message.h"
class Session;
class ChatService
{
    public:
    static ChatService& instance();
    static void registerHandler();
    void PrivateChat(const Message& message,Session* session);
    
    private:
    ChatService() = default;
};