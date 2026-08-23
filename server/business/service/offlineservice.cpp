#include "offlineservice.h"
#include<iostream>
#include "../../../common/protocol/message.h"
#include "../../model/entity/offlinemessage.h"
#include"../../model/messagemodel.h"
#include <chrono>
#include "../../model/filemodel.h"
#include "../businessdispatcher/businessdispatcher.h"
#include "../../model/filereceivermodel.h"
#include "../../model/friendmodel.h"
#include "../../model/groupmodel.h"
#include "../../model/usermodel.h"
OfflineService& OfflineService::instance()
{
    static OfflineService service;
    return service;
}

void OfflineService::sendOfflineMessage(Session* se)
{
     if(se == nullptr)
        return;

    if(!se->authenticated())
        return;

    auto userSession = dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
        return;

    int userId = userSession->userid();

    // 私聊未读数量通知
    sendPrivateUnreadNotify(userId, se);
    // 群聊未读数量通知
    sendGroupUnreadNotify(userId, se);
    // 文件通知
    sendOfflineFile(se);
    sendOfflineFriendRequest(se);
    
}
void OfflineService::sendGroupUnreadNotify(int userId, Session* se)
{
    if(se == nullptr)
        return;

    OfflineMessageModel offlineModel;
    MessageModel messageModel;
    GroupModel groupModel;
    auto messages = offlineModel.findByUserId(userId);
    std::unordered_map<std::int64_t, int> groupUnreadCount;
    for(const auto& offline : messages)
    {
        auto message = messageModel.findById(offline.messageId());

        if(!message)
            continue;
        if(message->groupId() == 0)
            continue;

        groupUnreadCount[message->groupId()]++;
    }

    for(const auto& [groupId, count] : groupUnreadCount)
    {
        auto group = groupModel.findById(groupId);

        if(!group)
            continue;

        Message notice;
        notice.setType(Messagetype::GroupUnreadNotify);
        notice.setReceiverId(userId);
        notice.payload()["groupId"] = groupId;
        notice.payload()["groupName"] = group->name();
        notice.payload()["unreadCount"] = count;

        se->send(notice);
    }
}
void OfflineService::sendPrivateOfflineMessages( int userId, int friendId,Session* se)
{
    if(!se || !se->authenticated())
        return;

    OfflineMessageModel model;
    auto messages = model.findPrivateMessages(userId, friendId);
    for(const auto& offline : messages)
    {
        sendChatOffline(offline, se);
    }
    model.clearPrivateMessages(userId, friendId);
}
void OfflineService::sendGroupOfflineMessages( int userId, int64_t groupId, Session* se)
{
    if(!se || !se->authenticated())
        return;
    OfflineMessageModel model;
    auto messages = model.findGroupMessages(userId, groupId);
    for(const auto& offline : messages)
    {
        sendGroupOffline(offline, se);
    }

    model.clearGroupMessages(userId, groupId);
}
void OfflineService::sendPrivateUnreadNotify(int userId, Session* se)
{
    if(se == nullptr)
        return;

    OfflineMessageModel offlineModel;
    MessageModel messageModel;
    UserModel userModel;

    auto messages = offlineModel.findByUserId(userId);
    std::unordered_map<int, int> privateUnreadCount;
    for( auto& offline : messages)
    {
        auto message = messageModel.findById(offline.messageId());

        if(!message)
            continue;
        if(message->groupId() != 0)
            continue;
        int senderId = message->sendId();
        privateUnreadCount[senderId]++;
    }
    for( auto& [friendId, count] : privateUnreadCount)
    {
        auto user = userModel.findById(friendId);
        if(!user)
            continue;
        Message notice;
        notice.setType(Messagetype::PrivateUnreadNotify);
        notice.setReceiverId(userId);
        notice.payload()["friendId"] = friendId;
        notice.payload()["userName"] = user->username();
        notice.payload()["unreadCount"] = count;

        se->send(notice);
    }
}
void OfflineService::sendOfflineFriendRequest(Session* se)
{
   if(se == nullptr)
        return;

  auto userSe=dynamic_cast<UserSession*>(se);
  if(userSe==nullptr)
  {
    return;
  }
  int userId=userSe->userid();


    OfflineMessageModel offlinemModel;
    auto offmessage=offlinemModel.findByUserId(userId);
    for(auto& mes:offmessage)
    {
      if(mes.type()!=OfflineType::FriendRequest)
      continue;

      Message notice;
      notice.setType(Messagetype::FriendRequestNotify);
      notice.payload()["message"]="收到新的好友申请";
      se->send(notice);
       offlinemModel.remove(mes.id());
    }
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

   UserModel userModel;
    auto user = userModel.findById(message->sendId());

      Message reply;
   reply.setType(Messagetype::GroupChat);
   reply.setSenderId(message->sendId());
  
   auto timestamp=std::chrono::duration_cast<std::chrono::seconds>(message->sendTime().time_since_epoch()).count();
   reply.setTimestamp(timestamp);
   reply.payload()["groupId"]=message->groupId();
   reply.payload()["content"]=message->content();
    reply.payload()["offline"] = true;
   
    reply.payload()["senderName"] = user->username();
    
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
   FriendModel friendmodel;
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
    if(file->groupId()==0)
    {
      if(!friendmodel.isFriend(userid,file->senderId()))
      {
         continue;
      }
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
