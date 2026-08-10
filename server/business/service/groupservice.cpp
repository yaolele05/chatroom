#include "groupservice.h"
#include "../businessdispatcher/businessdispatcher.h"
#include "../../session/usersession.h"
#include "../../model/entity/group.h"
#include "../../model/entity/groupmember.h"
#include "../../model/groupmodel.h"
#include "../../model/messagemodel.h"
#include "../../model/offlinemodel.h"
#include"../../session/sessionmanager.h"
#include "../../model/offlinemodel.h"
#include <iostream>
GroupService& GroupService::instance()
{
    static GroupService service;
    return service;
}
void GroupService::rigisterHandler()
{
    auto& dispatcher=BusinessDispatcher::instance();
       dispatcher.registerHandler(Messagetype::CreateGroup,[](const Message& message,Session* session)
{
    GroupService::instance().createGroup(message,session);
});
   dispatcher.registerHandler(Messagetype::JoinGroup,[](const Message& message,Session* session)
{
    GroupService::instance().joinGroup(message,session);
});

  dispatcher.registerHandler(Messagetype::LeaveGroup,[](const Message& message,Session* session)
{
    GroupService::instance().leaveGroup(message,session);
});
   dispatcher.registerHandler(Messagetype::GroupChat,[](const Message& message,Session* session)
{
    GroupService::instance().groupChat(message,session);
});

  dispatcher.registerHandler(Messagetype::GroupList,[](const Message& message,Session* session)
{
    GroupService::instance().groupList(message,session);
});
}
void GroupService::createGroup(const Message& msg,Session*se)
{
    if(!se)
    return;
    if(!se->authenticated())
    return;
    auto userSession=dynamic_cast<UserSession*>(se);
    if(!userSession)
    return;

    int userId=userSession->userid();
    auto& payload=msg.payload();
    if(!payload.contains("groupName"))
    return;
    if(!payload.contains("description"))
    return;

    Group group;
    group.setOwnerId(userId);
    group.setName(payload["groupName"]);
    group.setDescription(payload["description"]);
    group.setCreateTime(std::chrono::system_clock::now());

    GroupModel model;
    bool ok = model.create(group);
    if(!ok)
    return;

    GroupMember member;
    member.setGroupId(group.id());
    member.setUserId(userId);
    member.setRole(GroupRole::Owner);
    member.setCreateTime(std::chrono::system_clock::now());
     
    ok=model.addGroupMember(member);
     
     Message reply;

     reply.setSenderId(0);
     reply.setReceiverId(userId);
     reply.setType(Messagetype::CreateGroupResponse);
     reply.setSequence(msg.sequence());
     reply.payload()["code"] =  ok ? 0 : -1;
     reply.payload()["message"] = ok ? "success" : "failed";
     
     if(ok)
     {
         reply.payload()["groupId"] =group.id();
     }
    se->send(reply);

}
void GroupService::joinGroup(const Message& msg,Session*se)
{
     if(!se)
    return;
    if(!se->authenticated())
    return;

    auto userSession = dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
        return;
    int userId = userSession->userid();

    auto& payload = msg.payload();

    if(!payload.contains("groupId"))
        return;
    int groupId = payload.at("groupId").get<int>();
    GroupModel model;
    auto group = model.findById(groupId);

    Message reply;
    reply.setType(Messagetype::JoinGroupResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(userId);

    if(!group)
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "group not exist";
        se->send(reply);
        return;
    }
    if(model.isGroupMember(groupId,userId))
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "already joined";
        se->send(reply);
        return;
    }

    GroupMember member;
    member.setGroupId(groupId);
    member.setUserId(userId);
    member.setRole(GroupRole::Member);
    member.setCreateTime(std::chrono::system_clock::now());

    bool ok = model.addGroupMember(member);

    reply.payload()["code"] = ok ? 0 : -1;
    reply.payload()["groupId"] = groupId;
    reply.payload()["message"] = ok ? "success" : "failed";

    se->send(reply);
}
void GroupService::leaveGroup(const Message& msg, Session* se)
{
    if(se == nullptr)
        return;
    if(!se->authenticated())
        return;

    auto userSession = dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
        return;

    int userId = userSession->userid();
    auto& payload = msg.payload();
    if(!payload.contains("groupId"))
        return;

    int groupId = payload.at("groupId").get<int>();
    GroupModel model;

    Message reply;
    reply.setType(Messagetype::LeaveGroupResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(userId);

    if(!model.isGroupMember(groupId,userId))
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "not group member";
        se->send(reply);
        return;
    }

    auto role = model.getGroupMemberRole(groupId,userId);
    std::cout << "leave group:"<< " userId=" << userId<< " groupId=" << groupId<< " role=" << static_cast<int>(role)<< std::endl;
    if(role == GroupRole::Owner)
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "owner can't leave";

        se->send(reply);
        return;
    }

    bool ok = model.removeGroupMember(groupId,userId);

    if(ok)
   {
    auto members = model.findGroupMembers(groupId);

    if(members.empty())
    {
        model.removeGroup(groupId);
    }
    }
    reply.payload()["code"] = ok ? 0 : -1;
    reply.payload()["groupId"] = groupId;
    reply.payload()["message"] = ok ? "success" : "failed";

    se->send(reply);
}
void GroupService::groupChat(const Message& msg, Session* se)
{
    if(se == nullptr)
        return;
    if(!se->authenticated())
        return;

    auto userSession = dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
        return;
    int userId = userSession->userid();
    auto& payload = msg.payload();

   if(!payload.contains("groupId"))
    return;

   if(!payload.contains("content"))
    return;
   int groupId = payload.at("groupId").get<int>();
  std::string content =payload.at("content").get<std::string>();
   
   GroupModel groupModel;    
   auto group = groupModel.findById(groupId);

  if(!group)
  {
    Message reply;
    reply.setType(Messagetype::Error);
    reply.setSequence(msg.sequence());
    reply.payload()["code"]=-1;
    reply.payload()["message"]="group not exist";

    se->send(reply);

    return;
   }
  if(!groupModel.isGroupMember(groupId,userId))
  {
    Message reply;

    reply.setType(Messagetype::Error);
    reply.setSequence(msg.sequence());
    reply.payload()["code"] = -1;
    reply.payload()["message"] =
        "not group member";
    se->send(reply);

    return;
   }

   ChatMessage chat;
   chat.setSendId(userId);
   chat.setGroupId(groupId);
   chat.setContent(content);
   chat.setSendTime(std::chrono::system_clock::now());

   MessageModel messageModel;
   if(!messageModel.insert(chat))
   {
    return;
   }
   auto members=groupModel.findGroupMembers(groupId);
   Message forward;
   forward.setType(Messagetype::GroupChat);
   forward.setSequence(msg.sequence());
   forward.setSenderId(userId);
   forward.setTimestamp(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

    forward.payload()["groupId"] = groupId;
   forward.payload()["content"] = content;
   forward.payload()["nickname"]=userSession->username();
   OfflineMessageModel offlineModel;
   for(const auto& member:members)
   {
    int userid=member.userId();
    if(userid==userId)
    {
        continue;
    }

    auto session=SessionManager::instance().getSession(userid);
    if(session)
    {
        session->send(forward);
    }
    else
    {
        OfflineMessage offline;
        offline.setUserId(userid);
        offline.setMessageId(chat.id());
        offline.setType(OfflineType::ChatMessage);
        offline.setCreateTime(std::chrono::system_clock::now());

        offlineModel.insert(offline);
    }
   }

   Message ack;
   ack.setType(Messagetype::MessageAck);
   ack.setSequence(msg.sequence());
   ack.payload()["code"] = 0;
   ack.payload()["message"] = "success";

     se->send(ack);

}
void GroupService::groupList(const Message& msg,Session* se)
{
  if(se==nullptr)
  return;
  if(!se->authenticated())
  return;

  auto userSession=dynamic_cast<UserSession*>(se);
  if(userSession ==nullptr)
  return;

  int userId=userSession->userid();
  GroupModel model;
  auto groups=model.findUserGroups(userId);
  std::cout<<"userid="<<userId<<" group count="<<groups.size()<<std::endl;
  for(auto& group:groups)
  {
   std::cout<<"id="<<group.id()<<" name="<<group.name()<<std::endl;
}

  Message reply;
  reply.setType(Messagetype::GroupListResponse);
  reply.setSequence(msg.sequence());
  reply.setReceiverId(userId);
  reply.payload()["code"]=0;
  auto& list=reply.payload()["groups"];

  for(auto& group:groups)
  {
    nlohmann::json item;
    item["groupId"]=group.id();
    item["name"]=group.name();
    item["description"]=group.description();
    item["ownId"]=group.OwnerId();

    list.push_back(item);
  }

    se->send(reply);
}