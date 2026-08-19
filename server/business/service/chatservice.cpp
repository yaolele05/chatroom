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
#include "friendservice.h"
using json=nlohmann::json;
ChatService& ChatService::instance()
{
   static ChatService service;
   return service;
}
void ChatService::registerHandler()
{
   
   BusinessDispatcher::instance().registerHandler(Messagetype::PrivateChat,[](const Message& message,Session*session)
      {
         ChatService::instance().PrivateChat(message,session);
      }
   );
    
   BusinessDispatcher::instance().registerHandler(Messagetype::PrivateChatRead,[](const Message& message,Session*session)
      {
         ChatService::instance().PrivateChatRead(message,session);
      }
   );
    BusinessDispatcher::instance().registerHandler( Messagetype::PrivateUnreadRequest,[](const Message& message,Session* session)
      {
          if (!session)
            return;
        auto userSession =SessionManager::instance().getSession(session->connection().get()           );
        if (!userSession)
            return;
        ChatService::instance().handlePrivateUnreadRequest(message, userSession);
         
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
   //std::cout<< "receiver="<< receiver.get()<< std::endl;
      OfflineMessageModel offlineModel;
      OfflineMessage offline;
      offline.setUserId(receiverId);
      offline.setMessageId(chat.id());
      offline.setType(OfflineType::ChatMessage);
      offline.setCreateTime(std::chrono::system_clock::now());

      bool ok = offlineModel.insert(offline);
      std::cout<< "offline insert="<< ok<< " messageId="<< chat.id()<< std::endl;
   if(receiver)
   {
      receiver->send(reply);
    
   }
 
   session->send(reply);

   Message ack;
   ack.setType(Messagetype::MessageAck);
   ack.setSequence(message.sequence());
   ack.setSenderId(0);          
   ack.setReceiverId(sendId);   
   ack.payload()["code"]=0;
   ack.payload()["msg"]="success";
   session->send(ack);
  

}
void ChatService::PrivateChatRead(  const Message& message,Session* session)
{
    if(session == nullptr)
        return;

    if(!session->authenticated())
        return;
    auto userSession = dynamic_cast<UserSession*>(session);
    if(userSession == nullptr)
        return;
    int userId = userSession->userid();
    int friendId = message.receiverId();
    if(friendId <= 0)
        return;

    OfflineMessageModel model;
    bool ok = model.clearPrivateMessages(userId,friendId);
    std::cout<< "[ChatService] mark private chat read" << " userId=" << userId<< " friendId=" << friendId << " result=" << ok  << std::endl;
}

void ChatService::handlePrivateUnreadRequest(const Message& msg,const std::shared_ptr<Session>& session)
{
    if(session == nullptr)
        return;

    if(!session->authenticated())
        return;

    auto userSession = dynamic_cast<UserSession*>( session.get());
    if(userSession == nullptr)
        return;
     uint32_t userId = userSession->userid();
    uint32_t peerId = msg.payload().value("peerId", 0);
     
     if(peerId == 0)
        return;

  
    Message checkMsg=msg;
    checkMsg.payload()["peerId"]=peerId;
    if(FriendService::instance().isFriendBlocked(checkMsg,session.get()))
    {
          std::cout
        << "[PrivateUnreadRequest]"
        << " userId=" << userId
        << " peerId=" << peerId
        << std::endl;

        Message response;
        response.setType(Messagetype::PrivateChatResponse);
        response.setSenderId(userId);
        response.payload()["code"]=-1;
        response.payload()["block"]=true;
        response.payload()["message"]="存在屏蔽关系，无法聊天";
        session->send(response);

        return;

    }

    
    OfflineMessageModel offlineModel;

    auto offlineMessages =offlineModel.findPrivateMessages(userId,peerId);

    std::cout<< "[PrivateUnreadRequest]" << " unread count="<< offlineMessages.size()<< std::endl;
  
    MessageModel messageModel;

    for(const auto& offline : offlineMessages)
    {
        auto message =
            messageModel.findById(
                offline.messageId()
            );

        if(!message)
        {
            std::cerr
                << "[PrivateUnreadRequest] "
                << "message not found, messageId="
                << offline.messageId()
                << std::endl;

            continue;
        }

        Message reply;

        reply.setType(
            Messagetype::PrivateChatResponse
        );

        reply.setSenderId(
            message->sendId()
        );

        reply.setReceiverId(
            message->receiverId()
        );

        reply.payload()["content"] =
            message->content();

        reply.payload()["messageId"] =
            message->id();

        reply.payload()["unread"] = true;

        session->send(reply);
    }


     Message done;

    done.setType(Messagetype::PrivateChatResponse);
    done.setSenderId(peerId);
    done.setReceiverId(userId);
    done.payload()["peerId"] = peerId;
    done.payload()["unreadDone"] = true;
     session->send(done);
}