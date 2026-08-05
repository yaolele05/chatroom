#pragma once
#include <string>
#include "tcpclient.h"
#include "../common/protocol/message.h"
#include <memory>

class EventLoop;
class ClientConnection;
class Client
{
    public:
    Client(EventLoop* loop);

    bool connect(const std::string& ip,uint16_t port);
   void login(const std::string& username,const std::string& password);
   void registerUser(const std::string& username,const std::string& password);
   void sendPrivateMessage(int userid,const std::string& text);
   void sendGroupMessage(int64_t groupid,const std::string& text);


    private:
   void onMessage(const Message& message);
   private:
   EventLoop* loop_;
   std::unique_ptr<TcpClient> tcpClient_;
   std::unique_ptr<ClientConnection> clientConn_;
};