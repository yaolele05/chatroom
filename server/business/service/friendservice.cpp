#include "friendservice.h"
#include "../businessdispatcher/businessdispatcher.h"
#include"../../session/session.h"
#include"../../session/usersession.h"
#include "../../model/entity/friend.h"
#include "../../model/friendmodel.h"
#include "../../model/usermodel.h"
#include "../../database/connectionpool/redispool.h"
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

     Friend rela;
     rela.setUserId(userId);
     rela.setFriendId(friendId);
     rela.setStatus(0);
     rela.setCreateTime(std::chrono::system_clock::now());

     FriendModel model;
     bool ok=model.addFriend(rela);
    
     Message reply;
     reply.setType(Messagetype::AddFriendResponse);
     reply.setSequence(msg.sequence());
     reply.setReceiverId(userId);
     reply.payload()["code"]=ok? 0:-1;
     reply.payload()["friendId"]=friendId;
     reply.payload()["message"]= ok? "success":"failed";

     se->send(reply);

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
        if(redis)
        {
            online=redis->isUserOnline(re.friendId());
        }
        nlohmann::json item;

        item["id"]=user->id();
        item["username"]=user->username();
        item["nickname"]=user->nickname();
        item["avatar"]=user->avatar();
        item["signature"]=user->signature();
        item["status"]=re.status();

      payload["friends"].push_back(item);
          
     }
    RedisPool::instance().releaseConnection(redis);
      se->send(reply);

}