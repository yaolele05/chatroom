#include "friendservice.h"
#include "../businessdispatcher/businessdispatcher.h"
#include"../../session/session.h"
#include"../../session/usersession.h"
#include "../../model/entity/friend.h"
#include "../../model/friendmodel.h"
#include "../../model/usermodel.h"
#include "../../model/offlinemodel.h"
#include "../../database/connectionpool/redispool.h"
#include<iostream>
#include "../../../common/protocol/Jsoncodec.h"
#include "../../session/sessionmanager.h"
FriendService& FriendService::instance()
{
    static FriendService service;
    return service;
}
void FriendService::registerHandler()
{
    auto& dispatcher=BusinessDispatcher::instance();
   dispatcher.registerHandler(Messagetype::AddFriend,[](const Message& message,Session* session)
{
    FriendService::instance().addFriend(message,session);
});

   dispatcher.registerHandler(Messagetype::DeleteFriend,[](const Message& message,Session* session)
{
    FriendService::instance().deleteFriend(message,session);
});
   dispatcher.registerHandler(Messagetype::FriendList,[](const Message& message,Session* session)
{
   FriendService::instance().FriendList(message,session);   
});
 dispatcher.registerHandler(Messagetype::BlockFriend,[](const Message&msg,Session* se)
{
   FriendService::instance().blockFriend(msg,se);   
});
  dispatcher.registerHandler(Messagetype::UnblockFriend,[](const Message&msg,Session* se)
{
   FriendService::instance().unblockFriend(msg,se);   
});
 dispatcher.registerHandler(Messagetype::AcceptFriend,[](const Message&msg,Session* se)
{
   FriendService::instance().acceptFriend(msg,se);   
});
 dispatcher.registerHandler(Messagetype::RejectFriend,[](const Message&msg,Session* se)
{
   FriendService::instance().rejectFriend(msg,se);   
});
 dispatcher.registerHandler(Messagetype::FriendRequestList,[](const Message&msg,Session* se)
{
   FriendService::instance().friendRequestList(msg,se);   
});
}
void FriendService::addFriend(const Message& msg,Session* se)
{
    if(se==nullptr)
    return;
    if(!se->authenticated())
    return;
    auto userSession=dynamic_cast<UserSession*>(se);

      if(userSession==nullptr)
      return;

      int userId=userSession->userid();

      auto& payload=msg.payload();
      if(!payload.contains("friendId"))
      return;

     int friendId=payload.at("friendId").get<int>();
     uint32_t fromUserId=userId;
     uint32_t toUserId=friendId;
     
       if(userId==friendId)
     {
         Message reply;
      reply.payload()["message"]="cannot add yourself";
       se->send(reply);
       return;
     }  
      std::cout<<"add friend request:"<<"userid="<<userSession->userid()<<" friendid="<<friendId<<std::endl;

     
     FriendModel model;
     UserModel userModel;

     auto user = userModel.findById(friendId);

    if(!user)
    {
    Message reply;

    reply.setType(Messagetype::AddFriendResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(userId);

    reply.payload()["code"] = -1;
    reply.payload()["friendId"] = friendId;
    reply.payload()["message"] = "用户不存在";

    se->send(reply);

    return;
    }
     if(model.isFriend(userId,friendId))
     {
    
     Message reply;
     reply.setType(Messagetype::AddFriendResponse);
     reply.setSequence(msg.sequence());
     reply.setReceiverId(userId);
     reply.payload()["code"]=-1;
     reply.payload()["friendId"]=friendId;
     reply.payload()["message"]= "已经是好友";
   
     se->send(reply);
     return;
     }
     bool ok=model.createFriendRequest(userId,friendId);
     Message reply;
    reply.setType(Messagetype::AddFriendResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(userId);
    reply.payload()["code"] =ok ? 0 : -1;
    reply.payload()["friendId"] =friendId;
    reply.payload()["message"] =ok ? "好友申请已发送" : "好友申请发送失败";
    se->send(reply);
    if(ok)
    {
        

        auto nse=SessionManager::instance().getSession(friendId);
    if(nse)
    {
    Message notice;
    notice.setType(Messagetype::FriendRequestNotify);
    notice.setSequence(0);
    notice.payload()["message"] = "收到一条新的好友申请";
    nse->send(notice);
     }
     else
     {
        OfflineMessage offline;
        offline.setUserId(friendId);
        offline.setType(OfflineType::FriendRequest);
        OfflineMessageModel offlinemodel;
        offlinemodel.insert(offline);
     }

   }
}
void FriendService:: deleteFriend(const Message& msg,Session* se)
{
     if(se==nullptr)
    return;
    if(!se->authenticated())
    return;

    auto userSession=dynamic_cast<UserSession*>(se);

      if(userSession==nullptr)
      return;

      int userId=userSession->userid();
      auto& payload=msg.payload();
      if(!payload.contains("friendId"))
      return;
     int friendId=payload.at("friendId").get<int>();
     FriendModel model;
     bool ok=model.removeFriend(userId,friendId);

     Message reply;
     reply.setType(Messagetype::DeleteFriendResponse);
     reply.setSequence(msg.sequence());
     reply.setReceiverId(userId);
     reply.payload()["code"]=ok? 0:-1;
     reply.payload()["friendId"]=friendId;
     reply.payload()["message"]= ok? "success":"failed";

     se->send(reply);

}
void FriendService::FriendList(const Message& msg,Session*se)
{
     if(se==nullptr)
     {
        return;
     }
     if(!se->authenticated())
     return;

     auto userSession=dynamic_cast<UserSession*>(se);
     if(userSession==nullptr)
     return;
     int userId=userSession->userid();

     FriendModel fmodel;
     auto rela=fmodel.findFriends(userId);
    
     UserModel userModel;
     OfflineMessageModel offlineModel;
     Message reply;
     reply.setType(Messagetype::FriendListResponse);
     reply.setSequence(msg.sequence());
     reply.setReceiverId(userId);
     
     auto& payload=reply.payload();
     payload["friends"]=nlohmann::json::array();

     auto redis = RedisPool::instance().getConnection();
     for(const auto& re:rela)
     {
        auto user=userModel.findById(re.friendId());
        if(!user)
        {
            continue;
        }
        bool online=false;
        bool blocked = false;
         bool blockedByfriend=false;
        if(redis)
        {
            online=redis->isUserOnline(re.friendId());
            blocked=redis->isBlocked(userId,re.friendId());
           blockedByfriend=redis->isBlocked(re.friendId(),userId);

        }
        nlohmann::json item;
        item["id"]=user->id();
        item["username"]=user->username();
        item["nickname"]=user->nickname();
        item["avatar"]=user->avatar();
        item["signature"]=user->signature();
        item["status"]=re.status();
        item["online"] = online;
        item["blocked"]=blocked;
        item["blockedByFriend"]=blockedByfriend;
        int unreadCount=offlineModel.countPrivateUnread(userId,re.friendId());
         item["unreadCount"] = unreadCount;
      payload["friends"].push_back(item);
          
     }
    RedisPool::instance().releaseConnection(redis);

      se->send(reply);
}
bool FriendService::blockFriend(const Message& msg,Session* se)
{
    if(se==nullptr ||! se->authenticated())
    {
        return false;
    }
    
     auto userSession = dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
    {
        return false;
    }

    uint32_t userId = userSession->userid();

    const auto& payload = msg.payload();
    uint32_t friendId = payload.value("friendId", 0u);

    Message response;
    response.setType(Messagetype::BlockFriendResponse);
    response.setSequence(msg.sequence());
    response.setSenderId(userId);

    if(userId==friendId)
    {
          response.payload()["code"]=-1;
        response.payload()["message"]="不能屏蔽自己";
        se->send(response);
        return false;
    }
    auto conn=RedisPool::instance().getConnection();
    if(!conn)
    {

        response.payload()["code"]=-1;
        response.payload()["message"]="redis 连接失败";
        se->send(response);
        
        return false;
    }

    bool ok=conn->blockFriend(userId,friendId);
    RedisPool::instance().releaseConnection(conn);
    response.payload()["code"]=ok? 0:1;
    response.payload()["friendId"]=friendId;
  
    response.payload()["message"]=ok? "屏蔽成功" : "屏蔽失败";
    se->send(response);
    return ok;
}
bool FriendService::unblockFriend(const Message& msg,Session* se)
{
    
    if(se==nullptr ||! se->authenticated())
    {
        return false;
    }
    
     auto userSession = dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
    {
        return false;
    }

    uint32_t userId = userSession->userid();

    const auto& payload = msg.payload();
    uint32_t friendId = payload.value("friendId", 0u);

     Message response;
    response.setType(Messagetype::UnblockFriendResponse);
    response.setSequence(msg.sequence());
    response.setSenderId(userId);

    if(userId==friendId)
    {
        response.payload()["code"]=-1;
        response.payload()["message"]="不能屏蔽自己";
        se->send(response);
        return false;
    }
    auto conn=RedisPool::instance().getConnection();
    if(!conn)
    {

        response.payload()["code"]=-1;
        response.payload()["message"]="redis 连接失败";
        se->send(response);
        return false;
    }
    
    bool ok=conn->unblockFriend(userId,friendId);
    RedisPool::instance().releaseConnection(conn);
    response.payload()["code"]=ok? 0:1;
    response.payload()["friendId"]=friendId;
  
    response.payload()["message"]=ok? "取消屏蔽成功" : "取消屏蔽失败";
    se->send(response);
    return ok;
}
bool FriendService::isFriendBlocked(const Message& msg, Session* se)
{
    if(se == nullptr || !se->authenticated())
    {
        return false;
    }

    auto userSession = dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
    {
        return false;
    }

    uint32_t userId = userSession->userid();
    uint32_t friendId=0;

    const auto& payload = msg.payload();
    if(payload.contains("friendId"))
    {
        friendId=payload.value("friendId",0u);
    }
    else if (payload.contains("peerId"))
    {
        friendId=payload.value("peerId",0u);
    }
    else
    {
        friendId=msg.receiverId();
    }
    if(friendId==0 ||friendId==userId)
    {
        return false;
    }
    auto conn = RedisPool::instance().getConnection();

    if(!conn)
    {
        std::cerr<<"[FriendService] redis connection failed"<<std::endl;
        return false;
    }

    bool blockedByme= conn->isBlocked(userId, friendId);
    bool blockedByf=conn->isBlocked(friendId,userId);

    RedisPool::instance().releaseConnection(conn);

    return blockedByme ||blockedByf;
}
void FriendService::acceptFriend(const Message& msg,Session* se)
{
    if(se == nullptr || !se->authenticated())
        return;

    auto userSession = dynamic_cast<UserSession*>(se);

    if(userSession == nullptr)
        return;

    int userId = userSession->userid();
    const auto& payload = msg.payload();
    if(!payload.contains("requestId"))
        return;

    int requestId =  payload.at("requestId").get<int>();

    FriendModel model;
    auto request =model.findFriendRequest(requestId);

    if(!request)
    {
        return;
    }
    if(request->toUserId != userId)
    {
        return;
    }
    if(request->status != 0)
    {
        return;
    }

    // 再检查已经成为好友
    if(model.isFriend( request->fromUserId,request->toUserId))
    {
        return;
    }

    Friend rela;

    rela.setUserId(request->fromUserId);
    rela.setFriendId(request->toUserId);
    rela.setStatus(0);
    rela.setCreateTime( std::chrono::system_clock::now());

    bool ok = model.addFriend(rela);

    if(ok)
    {
        
        model.updatefRequestStatus(request->fromUserId,request->toUserId, 1);
    }

    Message reply;

    reply.setType(Messagetype::AcceptFriendResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(userId);

    reply.payload()["code"] = ok ? 0 : -1;
    reply.payload()["requestId"] = requestId;

    reply.payload()["message"] = ok ? "已接受好友申请" : "接受好友申请失败";

    se->send(reply);
}
void FriendService::rejectFriend(const Message& msg,Session* se)
{

     if(se == nullptr || !se->authenticated())
        return;

    auto userSession = dynamic_cast<UserSession*>(se);

    if(userSession == nullptr)
        return;

    int userId = userSession->userid();
    const auto& payload = msg.payload();
    if(!payload.contains("requestId"))
        return;

    int requestId =  payload.at("requestId").get<int>();

    FriendModel model;
    auto request =model.findFriendRequest(requestId);

    if(!request)
    {
        return;
    }
    if(request->toUserId != userId)
    {
        return;
    }
    if(request->status != 0)
    {
        return;
    }

    bool ok=model.updatefRequestStatus(request->fromUserId,request->toUserId,1);
    
    Message reply;
    reply.setType(Messagetype::RejectFriendResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(userId);

    reply.payload()["code"] = ok ? 0 : -1;
    reply.payload()["requestId"] = requestId;

    reply.payload()["message"] = ok ? "已拒绝好友申请" : "拒绝好友申请失败";

  se->send(reply);
}
void FriendService::friendRequestList(const Message& msg,Session* se)
{
    if(se == nullptr || !se->authenticated())
        return;
    auto userSession =dynamic_cast<UserSession*>(se);

    if(userSession == nullptr)
        return;
    int userId = userSession->userid();
    FriendModel model;
    auto requests =model.findPendingFriendRequest(userId);
    UserModel userModel;
    Message reply;
    reply.setType(Messagetype::FriendRequestListResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(userId);
    auto& payload = reply.payload();
    payload["requests"] = nlohmann::json::array();/////
    for(const auto& request : requests)
    {
        auto user =userModel.findById(request.fromUserId);

        if(!user)
            continue;
        nlohmann::json item;
        item["requestId"] = request.id;
        item["fromUserId"] =request.fromUserId;
        item["username"] = user->username();
        item["nickname"] = user->nickname();
        item["avatar"] = user->avatar();
        item["signature"] =user->signature();
        item["createTime"] =request.createTime;
        payload["requests"].push_back(item);
    }

    se->send(reply);
}