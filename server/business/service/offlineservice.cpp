#include "offlineservice.h"
#include<iostream>
#include "../../../common/protocol/message.h"
#include "../../model/entity/offlinemessage.h"
#include"../../model/messagemodel.h"
#include <chrono>
#include "../../model/filemodel.h"
#include "../businessdispatcher/businessdispatcher.h"
#include "../../model/filereceivermodel.h"
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
        
        success=sendChatOffline(offline,se);
       
        break;
       
        default:
        break;
        }
       
    }
    sendOfflineFile(se);
}
bool OfflineService::sendChatOffline(const OfflineMessage& offline,Session* se)
{
   MessageModel model;
   auto message=model.findById(offline.messageId());
   if(!message)
   return false;
  
   if(message->groupId()!=0)
   {
    sendGroupOffline(offline,se);
    return true;
   }
   Message reply;
   reply.setType(Messagetype::PrivateChat);
   reply.setSenderId(message->sendId());
   reply.setReceiverId(message->receiverId());
   auto timestamp=std::chrono::duration_cast<std::chrono::seconds>(message->sendTime().time_since_epoch()).count();
   reply.setTimestamp(timestamp);

   reply.payload()["content"]=message->content();
  

   se->send(reply);
   return true;
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
    reply.payload()["offline"] = true;
    reply.payload()["senderName"] ="用户" + std::to_string(message->sendId());
   se->send(reply);

}
void OfflineService::sendOfflineFile(Session*se)
{
    if(se==nullptr)
    return;
    if(!se->authenticated())
    return;
    auto userSession=dynamic_cast<UserSession*>(se);

    if(!userSession)
    return;
    FileReceiverModel receiverModel;
      int userid=userSession->userid();
   std::cout<< "[OfflineFile] userid=" << userid<< std::endl;
   auto receivers =receiverModel.findWaitingFiles(userid);
  std::cout<<"offline file count="<<receivers.size()<<std::endl;
   FileModel fileModel;
   for(auto& receiver:receivers)
   {
     auto file =fileModel.findById(receiver.fileId());

     if(!file)
    {
    continue;
     }
    if(!file->completed())
    {
      continue;
    }
    Message msg;
    msg.setType(Messagetype::OfflineFileNotify);
    msg.setSenderId(file->senderId());
    msg.setReceiverId(userid);
    msg.payload()["fileId"]= file->id();
    msg.payload()["fileName"]= file->fileName();
    msg.payload()["fileSize"]= file->fileSize();
    msg.payload()["sha256"]= file->fileSha256();
    msg.payload()["groupId"]=file->groupId();
    std::cout<< "[SEND OfflineFileNotify] SOURCE=offlineservice"<< " fileId=" << file->id() << " receiverId=" << userid<< std::endl;
     se->send(msg);
   std::cout<<"send offline file notify "<<"fileId="<<file->id()<<std::endl;

   }
}