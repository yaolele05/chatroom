#include "loginservice.h"
#include "../businessdispatcher/businessdispatcher.h"
#include "../../session/session.h"
#include "../../session/usersession.h"
#include "../../session/sessionmanager.h"
#include "../../model/entity/user.h"
#include "../../model/usermodel.h"
#include "../../database/connectionpool/redispool.h"
#include "offlineservice.h"
#include "../../../common/security/crypto/sha256.h"
#include <iostream>
#include "../../session/sessionmanager.h"
LoginService& LoginService::instance()
{
    static LoginService instance;

    return instance;
}
void LoginService::registerHandle()
{
    std::cout << "register login handler" << std::endl;
    auto& dispatcher=BusinessDispatcher::instance();
    dispatcher.registerHandler(Messagetype::Register,[](const Message& message,Session* session)
{
    LoginService::instance().registerUser(message,session);
});
     dispatcher.registerHandler(Messagetype::Login,[](const Message& message,Session* session)
{
    LoginService::instance().login(message,session);
});
 dispatcher.registerHandler(Messagetype::Logout,[](const Message& message,Session* session)
{
    LoginService::instance().logout(message,session);
});
}
void LoginService::registerUser(const Message& msg,Session* se)
{
    if(se==nullptr)
    return;

    auto& payload=msg.payload();
    if(!payload.contains("username"))
    return;
    if(!payload.contains("password"))
    return;

    User user;
    user.setUsername(payload["username"].get<std::string>());
    user.setPasswordHash(Sha256::data(payload["password"].get<std::string>()));
    user.setUpdateTime(std::chrono::system_clock::now());
    if(payload.contains("nickname"))
        user.setNickname(payload["nickname"].get<std::string>());
    else
        user.setNickname(payload["username"].get<std::string>());

    if(payload.contains("signature"))
        user.setSignature(payload["signature"]);
    user.setAvatar("");
     user.setCreateTime(std::chrono::system_clock::now());

    UserModel model;
    if(model.findByName(payload["username"]))
    {
        Message reply;
        reply.setType(Messagetype::RegisterResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "user already exists";

        se->send(reply);
        return;
    }
    bool ok = model.insert(user);
    std::cout<<"insert result"<<ok<<std::endl;

     Message reply;
    reply.setType(Messagetype::RegisterResponse);
    reply.setSequence(msg.sequence());

    reply.payload()["code"] = ok ? 0 : -1;
    reply.payload()["message"] = ok ? "success" : "failed";

    if(ok)
        reply.payload()["userId"] = user.id();

    if(!ok)
        reply.payload()["reason"] = "register failed";

    se->send(reply);

}
void LoginService::login(const Message& msg,Session* se)
{

     if(se == nullptr)
        return;
    auto userSession = dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
        return;

    auto& payload = msg.payload();
    if(!payload.contains("username"))
        return;

    if(!payload.contains("password"))
        return;

    UserModel model;
    std::cout<<"login username="<<payload["username"].get<std::string>()<<std::endl;
    auto user = model.findByName(payload["username"]);

    Message reply;
    reply.setType(Messagetype::LoginResponse);
    reply.setSequence(msg.sequence());
    
    if(!user)
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] = "user not exist";

        se->send(reply);
        return;
    }

    if(SessionManager::instance().contains(user->id()))
    {
    reply.payload()["success"] = false;
    reply.payload()["reason"] = "user already online";
    se->send(reply);
    return;
    }    
    std::string password =Sha256::data(payload["password"]);
     std::cout << "client password = "<< payload["password"].get<std::string>()<< std::endl;

    std::cout << "sha256(password) = "<< password<< std::endl;
    std::cout << "db password_hash = "<< user->passwordHash()<< std::endl;

    if(password != user->passwordHash())
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] = "password error";

        se->send(reply);
        return;
    }

    userSession->setUserid(user->id());
    userSession->setUsername(user->username());
    userSession->setAuthenticated(true);
    
    SessionManager::instance().bindUser(userSession);
    auto redis = RedisPool::instance().getConnection();
    if(redis)
    {
      if(!redis->setUserOnline(user->id()))
       {
       std::cout<<"redis set online failed"<<std::endl;
       }
      RedisPool::instance().releaseConnection(redis);
    }
    
    reply.payload()["success"] = true;
    reply.payload()["userid"] = user->id();
    reply.payload()["username"] = user->username();
    reply.payload()["nickname"] = user->nickname();
    reply.payload()["avatar"] = user->avatar();
    reply.payload()["signature"] = user->signature();

    se->send(reply);
   OfflineService::instance().sendOfflineMessage(userSession);
}
void LoginService::logout(const Message& msg,Session* se)
{
   if(se == nullptr)
        return;

    if(!se->authenticated())
        return;
    auto userSession = dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
        return;
    int userId = userSession->userid();
    auto redis = RedisPool::instance().getConnection();
    if(redis)
    {
      if(!redis->setUserOffline(userId))
      {
        std::cout<<"redis set offline failed\n";
      }
        RedisPool::instance().releaseConnection(redis);
    }


    userSession->setAuthenticated(false);
    SessionManager::instance().unbindUser(userSession);
    std::cout << "unbind user "<< se->userid()<< std::endl;

    Message reply;
    reply.setType(Messagetype::LogoutResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(userId);
    reply.payload()["success"] = true;
    reply.payload()["message"] = "logout success";

    se->send(reply);
}

