#include "offlineservice.h"
#include<iostream>
#include "../../../common/protocol/message.h"
#include "../../model/entity/offlinemessage.h"
#include"../../model/messagemodel.h"
#include <chrono>
#include "../../model/filemodel.h"
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
        switch(offline.type())
        {
        case OfflineType::ChatMessage:
        sendChatOffline(offline,se);
        break;
        case OfflineType::File:
        sendFileOffline(offline,se);
        break;
        default:
        break;
        }
        model.remove(offline.id());
    }
}
void::OfflineService::sendChatOffline(const OfflineMessage& offline,Session* se)
{
   MessageModel model;
   auto message=model.findById(offline.messageId());
   if(!message)
   return;
  
   Message reply;
   reply.setType(Messagetype::PrivateChat);
   reply.setSenderId(message->sendId());
   reply.setReceiverId(message->receiverId());
   auto timestamp=std::chrono::duration_cast<std::chrono::seconds>(message->sendTime().time_since_epoch()).count();
   reply.setTimestamp(timestamp);

   reply.payload()["content"]=message->content();

   se->send(reply);
}
void OfflineService::sendFileOffline(const OfflineMessage& offline,Session* se)
{
    FileModel model;
    auto file=model.findById(offline.messageId());
    if(!file)
    {
        return;
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
}