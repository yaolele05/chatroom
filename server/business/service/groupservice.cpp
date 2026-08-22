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
#include "../../model/usermodel.h"
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

 dispatcher.registerHandler(Messagetype::GroupMemberList,[](const Message& message,Session* session)
{
    GroupService::instance().groupMemberList(message,session);
});
dispatcher.registerHandler(Messagetype::GroupJoinRequestList,[](const Message& message,Session* session)
{
    GroupService::instance().groupJoinRequestList(message,session);
});
  dispatcher.registerHandler(Messagetype::AcceptGroupJoinRequest,[](const Message& message,Session* session)
{
    GroupService::instance().acceptJoinRequest(message,session);
});
  dispatcher.registerHandler(Messagetype::RejectGroupJoinRequest,[](const Message& message,Session* session)
{
    GroupService::instance().rejectJoinRequest(message,session);
});
   dispatcher.registerHandler(Messagetype::SetGroupAdmin,[](const Message& message,Session* session)
{
    GroupService::instance().setGroupRole(message,session);
});

 dispatcher.registerHandler(Messagetype::RemoveGroupMember,[](const Message& message,Session* session)
{
    GroupService::instance().removeGroupMember(message,session);
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
        reply.payload()["message"] = "群不存在";
        se->send(reply);
        return;
    }
    if(model.isGroupMember(groupId,userId))
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "已经加入群聊";
        se->send(reply);
        return;
    }

    if(model.hasPendingJoinRequest(groupId, userId))
   {
    reply.payload()["code"] = -1;
    reply.payload()["groupId"] = groupId;
    reply.payload()["message"] ="已经提交过入群申请";
    se->send(reply);
    return;
    }

    bool ok =model.createJoinRequest(groupId, userId);
    reply.payload()["code"] =ok ? 0 : -1;
    reply.payload()["groupId"] =groupId;
    reply.payload()["message"] =ok? "入群申请已发送" : "入群申请发送失败";
     se->send(reply);
   if(ok)
    {
       auto members = model.findGroupMembers(groupId);
       Message notice;
        notice.setType(Messagetype::GroupJoinRequestNotify);
       notice.setSequence(0);
       notice.setReceiverId(0);
     notice.payload()["groupId"] = groupId;
      notice.payload()["message"] = "收到一条新的入群申请";
      for(const auto& member : members)
     {
     auto role = member.role();
     if(role == GroupRole::Owner || role == GroupRole::Admin)
     {
        auto tar = SessionManager::instance().getSession(member.userId());
        if(tar)
        {
            tar->send(notice);
        }
      }
     }
    }
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
    reply.payload()["message"] = "not group member";
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
   forward.payload()["senderName"]=userSession->username();
   forward.payload()["offline"]=false;///实时消息通知
   OfflineMessageModel offlineModel;//这是未读未读未读
   for(const auto& member:members)
   {
    int userid=member.userId();

    auto session=SessionManager::instance().getSession(userid);
    if(session)
    {
        session->send(forward);
    }
    else
    {
    //只要是未读就记录
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
void GroupService::groupJoinRequestList(const Message& msg,Session* se)
{
    if(!se || !se->authenticated())
        return;
    auto userSession =dynamic_cast<UserSession*>(se);
    if(!userSession)
        return;

    int userId =userSession->userid();
    const auto& payload = msg.payload();
    if(!payload.contains("groupId"))
        return;

    int groupId = payload.at("groupId").get<int>();
    GroupModel model;
    Message reply;
    reply.setType(Messagetype::GroupJoinRequestListResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(userId);

    auto group =model.findById(groupId);

    if(!group)
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="群不存在";
        se->send(reply);
        return;
    }

  
    // 在群里的角色
    GroupRole role =model.getGroupMemberRole( groupId,userId);

    if(role != GroupRole::Owner &&role != GroupRole::Admin)
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="没有权限查看入群申请";
        se->send(reply);
        return;
    }

    auto requests =model.findPendingJoinRequests(groupId);

    reply.payload()["code"] = 0;
    reply.payload()["groupId"] = groupId;
    reply.payload()["requests"] = requests;

    se->send(reply);
}
void GroupService::acceptJoinRequest(const Message& msg, Session* se)
{
  
    if(se == nullptr || !se->authenticated())
        return;
   

    auto userSession =dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
        return;

    int actId =userSession->userid();
    const auto& payload =msg.payload();
    if(!payload.contains("requestId"))
        return;
    std::int64_t requestId = payload.at("requestId").get<std::int64_t>();
    GroupModel model;

    bool ok = model.acceptJoinRequest(requestId,  actId);
   
    Message reply;
    reply.setType( Messagetype::AcceptGroupJoinRequestResponse);

    reply.setSequence(msg.sequence());
    reply.setReceiverId(actId);

    reply.payload()["code"] = ok ? 0 : -1;
    reply.payload()["requestId"] =requestId;
    reply.payload()["message"] =ok? "已接受入群申请":"接受入群申请失败";
    se->send(reply);
}
void GroupService::rejectJoinRequest(const Message& msg,   Session* se)
{
    if(se == nullptr || !se->authenticated())
        return;
    auto userSession =dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
        return;
    int operatorId =userSession->userid();

    const auto& payload =msg.payload();
    if(!payload.contains("requestId"))
        return;
    std::int64_t requestId =payload.at("requestId").get<std::int64_t>();

    GroupModel model;
    bool ok =model.rejectJoinRequest(requestId,operatorId);

    Message reply;
    reply.setType(Messagetype::RejectGroupJoinRequestResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(operatorId);
    reply.payload()["code"] =ok ? 0 : -1;
    reply.payload()["requestId"] =requestId;
    reply.payload()["message"] =ok? "已拒绝入群申请": "拒绝入群申请失败";

    se->send(reply);
}
void GroupService::setGroupRole(const Message& msg,Session* se)
{
    if(se == nullptr || !se->authenticated())
        return;

    auto userSession =dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
        return;
    int operatorId =userSession->userid();

    const auto& payload =msg.payload();

    if(!payload.contains("groupId") ||!payload.contains("userId") ||!payload.contains("role"))
    {
        return;
    }
    std::int64_t groupId =payload.at("groupId").get<std::int64_t>();

    int targetUserId =payload.at("userId").get<int>();
    int roleValue =payload.at("role").get<int>();

    Message reply;
    reply.setType( Messagetype::SetGroupAdminResponse  );

    reply.setSequence(msg.sequence());
    reply.setReceiverId(operatorId);
    if(roleValue !=static_cast<int>(GroupRole::Member) &&roleValue !=static_cast<int>(GroupRole::Admin))
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "非法角色";

        se->send(reply);
        return;
    }

    GroupModel model;
    auto group =model.findById(groupId);

    if(!group)
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="群不存在";

        se->send(reply);
        return;
    }
    GroupRole operatorRole = model.getGroupMemberRole(groupId,operatorId);

    if(operatorRole != GroupRole::Owner)
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="只有群主可以设置管理员";
        se->send(reply);
        return;
    }
    if(targetUserId == operatorId)
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "不能修改群主角色";
        se->send(reply);
        return;
    }
    if(!model.isGroupMember(groupId,targetUserId))
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="目标用户不是群成员";
        se->send(reply);
        return;
    }

    GroupRole tarRole =static_cast<GroupRole>(roleValue);
    bool ok =model.setGroupMemberRole(groupId,targetUserId,tarRole);

    reply.payload()["code"] =ok ? 0 : -1;
    reply.payload()["groupId"] =  groupId;
    reply.payload()["userId"] =targetUserId;
    reply.payload()["role"] =roleValue;

    reply.payload()["message"] =ok? (tarRole == GroupRole::Admin? "已设置为管理员": "已取消管理员"): "设置管理员失败";

    se->send(reply);
}
void GroupService::groupMemberList(const Message& msg,Session* se)
{
    if(!se || !se->authenticated())
        return;

    auto userSession =dynamic_cast<UserSession*>(se);
    if(!userSession)
        return;

    int userId = userSession->userid();
    const auto& payload = msg.payload();
    if(!payload.contains("groupId"))
        return;
    std::int64_t groupId =payload.at("groupId").get<std::int64_t>();

    GroupModel model;
    UserModel userModel;


    Message reply;
    reply.setType(Messagetype::GroupMemberListResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(userId);
    if(!model.findById(groupId))
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "群不存在";

        se->send(reply);
        return;
    }

    if(!model.isGroupMember(groupId,userId))
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "不是群成员";

        se->send(reply);
        return;
    }
    auto members =model.findGroupMembers(groupId);
    reply.payload()["code"] = 0;
    reply.payload()["groupId"] = groupId;
    auto& list =reply.payload()["members"];

    for(const auto& member : members)
    {
         int memberUserId = member.userId();
        nlohmann::json item;

        item["userId"] = member.userId();
        item["role"] =static_cast<int>(member.role());
        auto user = userModel.findById(memberUserId);
        if(user)
        {
            item["username"] = user->username();
            item["nickname"] = user->nickname();
        }
        else
        {
            item["username"] = "";
            item["nickname"] = "";
        }
        list.push_back(item);
    }

    se->send(reply);
}
void GroupService::removeGroupMember( const Message& msg, Session* se)
{
    if(!se || !se->authenticated())
        return;

    auto userSession =dynamic_cast<UserSession*>(se);
    if(!userSession)
        return;

    int operatorId = userSession->userid();
    const auto& payload = msg.payload();

    if(!payload.contains("groupId") ||!payload.contains("userId"))
    {
        return;
    }

    std::int64_t groupId = payload.at("groupId").get<std::int64_t>();
    int targetUserId =payload.at("userId").get<int>();
    Message reply;
    reply.setType(Messagetype::RemoveGroupMemberResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(operatorId);

    GroupModel model;

    auto group = model.findById(groupId);
    if(!group)
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "群不存在";
        se->send(reply);
        return;
    }

    if(!model.isGroupMember(groupId, operatorId))
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "不是群成员";

        se->send(reply);
        return;
    }

    GroupRole operatorRole =  model.getGroupMemberRole(groupId, operatorId);
    if(operatorRole != GroupRole::Owner)
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="只有群主可以移除成员";
        se->send(reply);
        return;
    }

    if(targetUserId == operatorId)
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "群主不能移除自己";
        se->send(reply);
        return;
    }

    if(!model.isGroupMember(groupId, targetUserId))
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "目标用户不是群成员";
        se->send(reply);
        return;
    }
    bool ok =model.removeGroupMember(groupId, targetUserId);
    reply.payload()["code"] = ok ? 0 : -1;
    reply.payload()["groupId"] = groupId;
    reply.payload()["userId"] = targetUserId;
    reply.payload()["message"] = ok ? "已移除群成员" : "移除群成员失败";
    se->send(reply);
}