#include "chatserver.h"
#include "session/sessionmanager.h"
#include "session/usersession.h"
#include "../common/protocol/message.h"
#include "../common/protocol/Jsoncodec.h"
#include "../common/protocol/packetcodec.h"
#include "database/connectionpool/redispool.h"
#include <iostream>
#include <functional>
Chatserver::Chatserver(EventLoop* loop, const InetAddress& addr):loop_(loop),server_(new TcpServer(loop, addr, "Chatserver"))
{ 

    server_->setConnectionCallback(std::bind(&Chatserver::onConnection,this,std::placeholders::_1));
    server_->setMessageCallback(std::bind(&Chatserver::onMessage,this,std::placeholders::_1,std::placeholders::_2));
}
void Chatserver::setThreadNum(int num)
{
    server_->setThreadNum(num);
}
void Chatserver::start()
{
    server_->start();
}
void Chatserver::onConnection(const TcpConnectionptr& conn)
{
   if(conn->connected())
    {
        auto session =std::make_shared<UserSession>( conn);
        SessionManager::instance().addSession(session);
        std::cout <<"new connection\n";
    }
    else
    {
        auto session=SessionManager::instance().getSession(conn.get());
        if(session)
        {
        auto redis =RedisPool::instance().getConnection();

        if(redis)
        {
            redis->setUserOffline(session->userid());RedisPool::instance().releaseConnection(redis);
        }
         }

        SessionManager::instance().removeSession(conn.get());
        std::cout<<"connection closed\n";
    }

}

void Chatserver::onMessage(const TcpConnectionptr& conn,Buffer* buffer)
{

    std::cout << "onMessage()" << std::endl;
    std::string json;
    while(true)
        {
        auto result = PacketCodec::decode(*buffer,json);
       // std::cout << "decode result=" << (int)result << std::endl;
        if(result == PacketCodec::DecodeResult::Needmoredata)
        {
            break;
        }
        if(result == PacketCodec::DecodeResult::ProtocolError)
        {
            std::cerr<<"protocol error\n";
            conn->forceClose();
            return;
        }
        std::cout << "decode ok" << std::endl;
        
         //std::cout << "raw json = " << json << std::endl;
        Message message;
        try
        {
           
            message = JsonCodec::decode(json);

        }
        catch(const std::exception& e)
        {
            std::cerr<<"json error:"<<e.what()<<std::endl;
            continue;
        }
        std::cout << "json ok" << std::endl;
        auto session =SessionManager::instance().getSession(conn.get());
        if(!session)
        {
            std::cerr<<"session not found\n";
            continue;
        }
        std::cout << "dispatch..." << std::endl;
        bool ok =BusinessDispatcher::instance().dispatch(message,session.get());

      if(!ok)
     {
       std::cout<<"no handler for message type="<<static_cast<int>(message.type())<<std::endl;
     }
          
    }

}
