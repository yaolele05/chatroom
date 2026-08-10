#include "offlineservice.h"
#include<iostream>
#include "../../../common/protocol/message.h"
#include "../../model/entity/offlinemessage.h"
#include"../../model/messagemodel.h"
#include <chrono>
#include "../../model/filemodel.h"
#include "../businessdispatcher/businessdispatcher.h"
OfflineService& OfflineService::instance()
{
    static OfflineService service;
    return service;
}

void OfflineService::sendOfflineMessage(Session* se)
{
    if(se==nullptr)
    return;
    if(!se->authenticated())
    return;

    auto userSession=dynamic_cast<UserSession*>(se);
    if(userSession==nullptr)
    return;

    int userid=userSession->userid();
    OfflineMessageModel model;
    auto messages=model.findByUserId(userid);
     std::cout << "offline count = " << messages.size()<< std::endl;
    for(auto& offline:messages)
    {
        bool success=false;
        switch(offline.type())
        {
        case OfflineType::ChatMessage:
        sendChatOffline(offline,se);
        success=true;
        break;
        case OfflineType::File:
        sendFileOffline(offline,se);
        success=true;
        break;
        default:
        break;
        }
        if(success)
        {
        model.remove(offline.id());
        }
    }
}
void::OfflineService::sendChatOffline(const OfflineMessage& offline,Session* se)
{
   MessageModel model;
   auto message=model.findById(offline.messageId());
   if(!message)
   return;
  
   if(message->groupId()!=0)
   {
    sendGroupOffline(offline,se);
    return;
   }
   Message reply;
   reply.setType(Messagetype::PrivateChat);
   reply.setSenderId(message->sendId());
   reply.setReceiverId(message->receiverId());
   auto timestamp=std::chrono::duration_cast<std::chrono::seconds>(message->sendTime().time_since_epoch()).count();
   reply.setTimestamp(timestamp);

   reply.payload()["content"]=message->content();

   se->send(reply);
}
void OfflineService::sendGroupOffline(const OfflineMessage& offline,Session* se)
{
    MessageModel model;
     auto message=model.findById(offline.messageId());
   if(!message)
   return;
      Message reply;
   reply.setType(Messagetype::GroupChat);
   reply.setSenderId(message->sendId());
  
   auto timestamp=std::chrono::duration_cast<std::chrono::seconds>(message->sendTime().time_since_epoch()).count();
   reply.setTimestamp(timestamp);
   reply.payload()["groupId"]=message->groupId();
   reply.payload()["content"]=message->content();

   se->send(reply);

}
bool OfflineService::sendFileOffline(const OfflineMessage& offline,Session* se)
{
    FileModel model;
    auto file=model.findById(offline.messageId());
    if(!file)
    {
        std::cout<<"find filebyID failed"<<std::endl;
        return false;
    }
    Message msg;
    msg.setType(Messagetype::FileFinish);
    msg.setSenderId(file->senderId());
    msg.setReceiverId(file->receiverId());
    auto timestamp=std::chrono::duration_cast<std::chrono::seconds>(file->createTime().time_since_epoch()).count();
    msg.setTimestamp(timestamp);

    msg.payload()["fileId"]=file->id();
    msg.payload()["fileName"]=file->fileName();
    msg.payload()["fileSize"]=file->fileSize();
    msg.payload()["sha256"]=file->fileSha256();

    se->send(msg);
    return true;
}