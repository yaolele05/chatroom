#include "chatserver.h"
#include<nlohmann/json.hpp>
#include <iostream>
#include "session/sessionmanager.h"
#include "business/businessdispatcher/businessdispatcher.h"
#include "protocol/protocol.h"
Chatserver::Chatserver(EventLoop* loop,const InetAddress& listenaddr):loop_(loop),server_(new TcpServer(loop,listenaddr,"Chatserver"))
{

    server_->setConnectionCallback(std::bind(&Chatserver::onConnection,this,std::placeholders::_1));
    server_->setMessageCallback(std::bind(&Chatserver::onMessage,this,std::placeholders::_1,std::placeholders::_2));
}
void Chatserver::start()
{
    server_->start();
}
void Chatserver::onConnection(const TcpConnectionptr& conn)
{
    if(conn->connected())
    {
        auto session=std::make_shared<UserSession>(conn);
        SessionManager::instance().addSession(session);
        std::cout<<"new connection"<<std::endl;
    }
    else
    {
        SessionManager::instance().removeSession(conn.get());
        std::cout<<"connection closed:"<<conn->peeraddress().port()<<std::endl;
    }
}
void Chatserver::onMessage(const TcpConnectionptr& conn,Buffer* buffer)
{
    std::string msg=buffer->retrieveAllAsString();
    json message;
    try
    {
        message=json::parse(msg);
    }
    catch(const std::exception& e)
    {
     conn->send(R"({"code“:400,”msg":"invalid json"})");   
     return;
    }
     
   auto session=SessionManager::instance().getSession(conn.get());
   if(!session)
   {
    conn->send(Protocol::makeError(401,"session not found"));
    return;
   }

   if(!BusinessDispatcher::instance().dispatch(message,session.get()))
   {
    
      conn->send(Protocol::makeError(401,"no session type "));
    
   }
}
