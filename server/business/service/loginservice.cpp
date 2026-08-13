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
#include <random>
LoginService& LoginService::instance()
{
    static LoginService instance;

    return instance;
}
void LoginService::registerHandle()
{
    //std::cout << "register login handler" << std::endl;
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

 dispatcher.registerHandler(Messagetype::SendRegisterCode, [](const Message& message, Session* session)
{
    LoginService::instance().sendRegisterCode(message, session);
});

 dispatcher.registerHandler(Messagetype::SendLoginCode,[](const Message& message, Session* session)
{
    LoginService::instance().sendLoginCode(message, session);
});

 dispatcher.registerHandler(Messagetype::SendResetCode,[](const Message& message, Session* session)
{
    LoginService::instance().sendResetCode(message, session);
});
}
void LoginService::registerUser(const Message& msg,Session* se)
{
    if(se==nullptr)
    return;

    auto& payload=msg.payload();
     std::string username =payload["username"].get<std::string>();
    std::string password =payload["password"].get<std::string>();
    std::string email =payload["email"].get<std::string>();

    if(!payload.contains("username"))
    return;
    if(!payload.contains("password"))
    return;
    if(!payload.contains("email"))
    {
        Message reply;
        reply.setType(Messagetype::RegisterResponse);
        reply.setSequence(msg.sequence());

        reply.payload()["code"] = -1;
        reply.payload()["message"] = "email required";

        se->send(reply);

    return;
    }
    
   
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
      if(model.findByName(payload["email"]))
    {
        Message reply;
        reply.setType(Messagetype::RegisterResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "email already exists";

        se->send(reply);
        return;
    }
    if(payload.contains("code"))
    {
        std::string code=payload["code"].get<std::string>();
        if(code.empty())
        {
            Message reply;
            reply.setType(Messagetype::RegisterResponse);
            reply.setSequence(msg.sequence());
            reply.payload()["code"]=-1;
            reply.payload()["message"]="需要验证码";
            se->send(reply);
            return;
        }
        bool ok=checkTheCode("register",email,code);
        if(!ok)
        {
            Message reply;
            reply.setType(Messagetype::RegisterResponse);
             reply.setSequence(msg.sequence());
            reply.payload()["code"]=-1;
            reply.payload()["message"]="验证码无效或过期\n";
            se->send(reply);
            return;
        }

    }
    User user;
    user.setUsername(username);
    user.setPasswordHash(Sha256::data(payload["password"].get<std::string>()));
    user.setEmail(email);
    user.setUpdateTime(std::chrono::system_clock::now());
     if(payload.contains("nickname"))
        user.setNickname(payload["nickname"].get<std::string>());
    else
        user.setNickname(payload["username"].get<std::string>());
    
    if(payload.contains("signature"))
        user.setSignature(payload["signature"]);
    user.setAvatar("");
 
    user.setCreateTime(std::chrono::system_clock::now());
  
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
std::string LoginService::thecode()
{
    static  std::random_device rd;
    static std::mt19937 gen(rd());////
    std::uniform_int_distribution<int> dist(100000,999999);
    return std::to_string(dist(gen));

}
bool LoginService::saveTheCode(const std::string& type,const std::string& email,const std::string& code)
{
    auto redis=RedisPool::instance().getConnection();
    if(!redis)
    {
        std::cout<<"[LoginService] get redis connection failed"<<std::endl;
        return false;
    }
    std::string key="check:"+type+":"+email;
    bool ok=redis->setex(key,80,code);
    RedisPool::instance().releaseConnection(redis);

    return ok;

}
bool LoginService::checkTheCode(const std::string& type,const std::string& email,const std::string& code)
{
    auto redis=RedisPool::instance().getConnection();
    if(!redis)
    {
        std::cout<<"[LoginService] get redis connection failed"<<std::endl;
        return false;
    }
     std::string key="check:"+type+":"+email;
     auto value=redis->get(key);
     if(!value)
     {
        RedisPool::instance().releaseConnection(redis);
        return false;
     }
     bool ok=(*value==code);
     if(ok)
     {
        redis->del(key);
     }
     RedisPool::instance().releaseConnection(redis);
     return ok;

}
void LoginService::sendRegisterCode(const Message& msg,Session* se)
{
    if(!se)
    return;
    auto& payload=msg.payload();
    if(!payload.contains("email"))
    {
      return;
    }
    std::string email=payload["email"].get<std::string>();

    if(email.empty())
    return;

    UserModel model;
    if(model.findByEmail(email))
    { 
        Message reply;
        reply.payload()["success"]=false;
        reply.payload()["reason"]="邮箱已经存在";
        se->send(reply);
        return;

    }

    std::string code=thecode();
    if(!saveTheCode("register",email,code))
    {
        Message reply;
        reply.setType(Messagetype::SendRegisterCodeResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["success"]=false;
        reply.payload()["reason"]="save code failed";
        se->send(reply);
        return;

    }
    //
       std::cout<<"[LoginService] 注册验证码："<<"邮箱："<<email<<"验证码："<<code<<std::endl;
    Message reply;
    reply.setType(Messagetype::SendRegisterCodeResponse);
    reply.setSequence(msg.sequence());
    reply.payload()["sucess"]=true;
    reply.payload()["reason"]="注册验证码发送";
    se->send(reply);
    return;
}
void LoginService::sendLoginCode(const Message& msg,Session* se)
{
    if(!se)
    {
        return;
    }
    auto& payload=msg.payload();
    if(!payload.contains("email"))
    {
        return;

    }
    std::string email=payload["email"].get<std::string>();

    if(email.empty())
    return;

    UserModel model;
    auto user=model.findByEmail(email);
    if(!user)
    {
        Message reply;
        reply.setType(Messagetype::SendLoginCodeResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["sucess"]=false;
        reply.payload()["reason"]="邮箱没有注册";
        se->send(reply);
        return;
    }
    std::string code=thecode();
    if(!saveTheCode("login",email,code))
    {
        Message reply;
        reply.setType(Messagetype::SendLoginCodeResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["sucess"]=false;
        reply.payload()["reason"]="验证码保存失败";
        se->send(reply);
        return;

    }
    std::cout<<"[LoginService] 登录验证码："<<"邮箱："<<email<<"验证码："<<code<<std::endl;
    Message reply;
    reply.setType(Messagetype::SendLoginCodeResponse);
    reply.setSequence(msg.sequence());
    reply.payload()["sucess"]=true;
    reply.payload()["reason"]="登录验证码发送";
    se->send(reply);
    return;
}
void LoginService::sendResetCode(const Message& msg,Session*se)
{
       if(!se)
    {
        return;
    }
    auto& payload=msg.payload();
    if(!payload.contains("email"))
    {
        return;

    }
    std::string email=payload["email"].get<std::string>();

    if(email.empty())
    return;

    UserModel model;
    auto user=model.findByEmail(email);
    if(!user)
    {
        Message reply;
        reply.setType(Messagetype::SendLoginCodeResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["sucess"]=false;
        reply.payload()["reason"]="邮箱没有注册";
        se->send(reply);
        return;
    }
    std::string code=thecode();

     if(!saveTheCode("reset",email,code))
    {
        Message reply;
        reply.setType(Messagetype::SendResetCodeResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["sucess"]=false;
        reply.payload()["reason"]="验证码保存失败";
        se->send(reply);
        return;

    }
     std::cout<<"[LoginService] 密码找回验证码："<<"邮箱："<<email<<"验证码："<<code<<std::endl;
    Message reply;
    reply.setType(Messagetype::SendResetCodeResponse);
    reply.setSequence(msg.sequence());
    reply.payload()["sucess"]=true;
    reply.payload()["reason"]="找回验证码发送";
    se->send(reply);
    return;
}
