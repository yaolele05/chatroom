#include "chatservice.h"
#include "../../model/entity/message.h"
#include "../../model/messagemodel.h"
#include "../../model/offlinemodel.h"
#include "../../session/sessionmanager.h"
#include "../../session/session.h"
#include <iostream>
#include <cerrno>
#include <nlohmann/json.hpp>
#include "../businessdispatcher/businessdispatcher.h"
#include <chrono>

using json=nlohmann::json;
ChatService& ChatService::instance()
{
   static ChatService service;
   return service;
}
void ChatService::registerHandler()
{
   
   BusinessDispatcher::instance().registerHandler(
      Messagetype::PrivateChat,[](const Message& message,Session*session)
      {
         ChatService::instance().PrivateChat(message,session);
      }
   );
}
void ChatService::PrivateChat(const Message& message,Session* session)
{
   if(session==nullptr)
   {
      return;
   }
   if(!session->authenticated())
   {
      return;
   }

   auto userSession=dynamic_cast<UserSession*>(session);
   if(userSession==nullptr)
   {
      return;
   }
   int sendId=userSession->userid();
   int receiverId=message.receiverId();
   auto& payload=message.payload();
   if(!payload.contains("content"))
   {
      return;
   }

   std::string content=payload.at("content").get<std::string>();
   std::cout<<"PrivateChat:"<<"sender="<<sendId<<" receiver="<<receiverId<<" content="<<content<<std::endl;
   
   ChatMessage chat;
   chat.setSendId(sendId);
   chat.setReceiverId(receiverId);
   chat.setContent(content);
   chat.setSendTime(std::chrono::system_clock::now());


   MessageModel model;
   if(!model.insert(chat))
   {
      std::cerr<<"savemessage failed"<<std::endl;
      return;
   }

   Message reply;
   reply.setType(Messagetype::PrivateChat);
   reply.setSequence(message.sequence());
   reply.setSenderId(sendId);
   reply.setReceiverId(receiverId);
   reply.setTimestamp(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

   reply.payload()["content"]=content;

   auto receiver =SessionManager::instance().getSession(receiverId);
   std::cout<< "receiver="<< receiver.get()<< std::endl;
   if(!receiver)
   {
      OfflineMessageModel offlineModel;
      OfflineMessage offline;
      offline.setUserId(receiverId);
      offline.setMessageId(chat.id());
      offline.setType(OfflineType::ChatMessage);
      offline.setCreateTime(std::chrono::system_clock::now());

      bool ok = offlineModel.insert(offline);
      std::cout<< "offline insert="<< ok<< " messageId="<< chat.id()<< std::endl;
    
   }
   else
   {
      std::cout<< "authenticated="<< receiver->authenticated() << std::endl;
      receiver->send(reply);
   }
   

   Message ack;
   ack.setType(Messagetype::MessageAck);
   ack.setSequence(message.sequence());
   ack.setSenderId(0);          
   ack.setReceiverId(sendId);   
   ack.payload()["code"]=0;
   ack.payload()["msg"]="success";
   session->send(ack);
  

}