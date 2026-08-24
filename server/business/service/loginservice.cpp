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
#include "../../database/connectionpool/mysqlpool.h"
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
 dispatcher.registerHandler(Messagetype::ResetPassword,[](const Message& message, Session* session)
{
    LoginService::instance().resetPassword(message, session);
});

 dispatcher.registerHandler(Messagetype::DeleteAccount,[](const Message& message, Session* session)
{
    LoginService::instance().deleteAccount(message, session);
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

     Message reply;
    reply.setType(Messagetype::RegisterResponse);
    reply.setSequence(msg.sequence());

    if(!payload.contains("username"))
    {
      
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "username required";
        se->send(reply);
       return;
    }
    
    if(!payload.contains("email"))
    {
       
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "email required";
        se->send(reply);
       return;
    }
    if(username.empty()||email.empty())
    {
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "username or email empty";
        se->send(reply);
       return;
    }
   
    UserModel model;
    if(model.findByName(username))
    {
       
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "user already exists";

        se->send(reply);
        return;
    }
      if(model.findByEmail(email))
    {
   
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "email already exists";

        se->send(reply);
        return;
    }
     bool passwordre=false;
     bool codere=false;
    

    if(payload.contains("password"))
    {
         password=payload["password"].get<std::string>();
        if(password.empty())
        {
         
            reply.payload()["code"]=-1;
            reply.payload()["message"]="需要密码";
            se->send(reply);
            return;
        }
        passwordre=true;
     

    }
    if(payload.contains("code"))
    {
        std::string code=payload["code"].get<std::string>();
        if(code.empty())
        {
            
            reply.payload()["code"]=-1;
            reply.payload()["message"]="需要验证码";
            se->send(reply);
            return;
        }
       
        bool ok=checkTheCode("register",email,code);
        if(!ok)
        {
           
            reply.payload()["code"]=-1;
            reply.payload()["message"]="验证码无效或过期\n";
            se->send(reply);
            return;
        }
         codere=true;

    }
    if(!passwordre && !codere)
    {
       reply.payload()["code"]=-1;
        reply.payload()["message"]="请输入密码或验证码\n";
        se->send(reply);
        return;
    }

    User user;
    user.setUsername(username);
   
    user.setEmail(email);
   
    if(passwordre)
    {
         user.setPasswordHash(Sha256::data(payload["password"].get<std::string>()));
    }
    else
    {
         user.setPasswordHash("");
    }
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
      Message reply;
    reply.setType(Messagetype::LoginResponse);
    reply.setSequence(msg.sequence());
    if(!payload.contains("loginType"))
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] = "需要logintype";

        se->send(reply);
        return;
    }
    std::string loginType=payload["loginType"].get<std::string>();
   

    UserModel model;
    std::optional<User> user;
   if(loginType =="password")
   {
    if(!payload.contains("username"))
    {
        reply.payload()["success"]=false;
        reply.payload()["reason"]="需要用户名";
        se->send(reply);
        return;
    }
     if(!payload.contains("password"))
    {
        reply.payload()["success"]=false;
        reply.payload()["reason"]="需要密码";
        se->send(reply);
        return;
    }
    std::string username=payload["username"].get<std::string>();
    std::string password =payload["password"].get<std::string>();
    if(username.empty()||password.empty())
    {
         reply.payload()["success"]=false;
        reply.payload()["reason"]="需要用户名或密码";
        se->send(reply);
        return;
    }
    user=model.findByName(username);
    if(!user)
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] = "user not exist";

        se->send(reply);
        return;
    }
 
     if(user->passwordHash().empty())
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] = "该账号没有密码，请使用邮箱验证码登录";

        se->send(reply);
        return;
    }
  
    std::string passwordHash =Sha256::data(password);

   std::cout << "[Login] calculated sha256="<< passwordHash << std::endl;

     std::cout << "[Login] database hash="<< user->passwordHash() << std::endl;
    if(passwordHash != user->passwordHash())
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] = "password error";

        se->send(reply);
        return;
    }

   }
    else if(loginType=="code")
    {
        if(!payload.contains("email"))
        {
            reply.payload()["success"]=false;
            reply.payload()["reason"]="email require";
            se->send(reply);
            return;
        }

        if(!payload.contains("code"))
        {
            reply.payload()["success"]=false;
            reply.payload()["reason"]="code require";
            se->send(reply);
            return;
        }
        std::string  email=payload["email"].get<std::string>();
        std::string  code=payload["code"].get<std::string>();
         if(email.empty()||code.empty())
       {
         reply.payload()["success"]=false;
        reply.payload()["reason"]="邮箱或验证码为空";
        se->send(reply);
        return;
       }
       user=model.findByEmail(email);
         if(!user)
       {
        reply.payload()["success"] = false;
        reply.payload()["reason"] = "user not exist";

        se->send(reply);
        return;
       }
       if(!checkTheCode("login",email,code))
       {
        reply.payload()["success"] = false;
        reply.payload()["reason"] = "验证码错误或过期";

        se->send(reply);
        return;
       }
        
    }
    else{
         reply.payload()["success"] = false;
        reply.payload()["reason"] = "unkown logintype";

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
        reply.setType(Messagetype::SendRegisterCodeResponse);
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
        reply.payload()["reason"]="验证码保存失败";
        se->send(reply);
        return;

    }
    //
       
    bool sent=EmailService::instance().send(email,"ChatRoom注册验证码","你的ChatRoom注册验证码是:"+code+"\r\n验证码80秒内有效，请勿泄漏给他人。");
    
    if(!sent)
    {
    Message reply;
    reply.setType(Messagetype::SendRegisterCodeResponse);
    reply.setSequence(msg.sequence());
    reply.payload()["success"]=false;
    reply.payload()["reason"]="邮件发送失败";
    se->send(reply);
    return;
    }
    Message reply;
    reply.setType(Messagetype::SendRegisterCodeResponse);
    reply.setSequence(msg.sequence());
    reply.payload()["success"]=true;
    reply.payload()["reason"]="注册验证码邮件发送成功";
    se->send(reply);
    return;
   
}
void LoginService::sendLoginCode(const Message& msg,Session* se)
{
    std::cout << "[LoginService] sendLoginCode start" << std::endl;
    if(!se)
    {
         std::cout << "[LoginService] session is null" << std::endl;
        return;
    }
    auto& payload=msg.payload();
    if(!payload.contains("email"))
    {

        std::cout << "[LoginService] email missing" << std::endl;
        return;

    }
    std::string email=payload["email"].get<std::string>();
        std::cout << "[LoginService] email=" << email << std::endl;
    if(email.empty())
    return;

    UserModel model;
    auto user=model.findByEmail(email);
    if(!user)
    {
        std::cout << "[LoginService] findByEmail..." << std::endl;
        Message reply;
        reply.setType(Messagetype::SendLoginCodeResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["success"]=false;
        reply.payload()["reason"]="邮箱没有注册";
        se->send(reply);
        return;
    }
    std::cout << "[LoginService] user found" << std::endl;

    std::string code=thecode();

    if(!saveTheCode("login",email,code))
    {
        std::cout << "[LoginService] save login code..." << std::endl;
        Message reply;
        reply.setType(Messagetype::SendLoginCodeResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["success"]=false;
        reply.payload()["reason"]="验证码保存失败";
        se->send(reply);
        return;

    }
    std::cout << "[LoginService] login code saved" << std::endl;
    bool sent=EmailService::instance().send(email,"ChatRoom登录验证码","你的ChatRoom登录验证码是:"+code+"\r\n验证码80秒内有效，请勿泄漏给他人。");
    
    if(!sent)
    {
        std::cout << "[LoginService] sending email..." << std::endl;
    Message reply;
    reply.setType(Messagetype::SendLoginCodeResponse);
    reply.setSequence(msg.sequence());
    reply.payload()["success"]=false;
    reply.payload()["reason"]="邮件发送失败";
    se->send(reply);
    return;
    }
   std::cout << "[LoginService] email sent, sending response" << std::endl;
    Message reply;
    reply.setType(Messagetype::SendLoginCodeResponse);
    reply.setSequence(msg.sequence());
    reply.payload()["success"]=true;
    reply.payload()["reason"]="登录验证码发送";
    se->send(reply);
    std::cout << "[LoginService] SendLoginCodeResponse sent" << std::endl;
    return;
}
void LoginService::sendResetCode(const Message& msg,Session*se)
{
      std::cout << "[LoginService] sendResetCode ENTER"
              << std::endl;

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
        reply.setType(Messagetype::SendResetCodeResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["success"]=false;
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
        reply.payload()["success"]=false;
        reply.payload()["reason"]="验证码保存失败";
        se->send(reply);
        return;

    }
    bool sent=EmailService::instance().send(email,"ChatRoom验证码","你的ChatRoom密码找回验证码是:"+code+"\r\n验证码80秒内有效，请勿泄漏给他人。");
    
    if(!sent)
    {
    Message reply;
    reply.setType(Messagetype::SendResetCodeResponse);
    reply.setSequence(msg.sequence());
    reply.payload()["success"]=false;
    reply.payload()["reason"]="邮件发送失败";
    se->send(reply);
    return;
    }
    
    Message reply;
    reply.setType(Messagetype::SendResetCodeResponse);
    reply.setSequence(msg.sequence());
    reply.payload()["success"]=true;
    reply.payload()["reason"]="找回验证码发送";
    se->send(reply);
    return;
}
void LoginService::resetPassword(const Message& msg,Session* se)
{
    if(se == nullptr)
        return;

    auto& payload = msg.payload();
    Message reply;
    reply.setType(Messagetype::ResetPasswordResponse);
    reply.setSequence(msg.sequence());

    if(!payload.contains("email") ||!payload.contains("code") ||!payload.contains("newPassword"))
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] =
        "email, code and newPassword required";
        se->send(reply);
        return;
    }

    std::string email = payload["email"].get<std::string>();
    std::string code =payload["code"].get<std::string>();
    std::string newPassword =payload["newPassword"].get<std::string>();

    if(email.empty() ||code.empty() ||newPassword.empty())
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] ="邮箱和验证码和新密码不能为空";

        se->send(reply);
        return;
    }

    if(newPassword.size() < 6)
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] ="密码至少6位";
        se->send(reply);
        return;
    }

    UserModel model;

    auto user = model.findByEmail(email);
    if(!user)
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] ="邮箱没有注册";
        se->send(reply);
        return;
    }
    if(!checkTheCode("reset", email, code))
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] ="验证码无效或已过期";
        se->send(reply);
        return;
    }
    user->setPasswordHash(Sha256::data(newPassword));
    user->setUpdateTime( std::chrono::system_clock::now());
    bool ok = model.update(*user);
    if(!ok)
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] ="密码修改失败";

        se->send(reply);
        return;
    }
    reply.payload()["success"] = true;
    reply.payload()["reason"] = "密码修改成功";

    se->send(reply);
}
void LoginService::deleteAccount(const Message& msg,Session* se)
{
    if(se == nullptr)
        return;

    if(!se->authenticated())
    {
        Message reply;

        reply.setType(Messagetype::DeleteAccountResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["success"] = false;
        reply.payload()["reason"] = "not logged in";

        se->send(reply);
        return;
    }
    auto userSession =dynamic_cast<UserSession*>(se);
    if(userSession == nullptr)
    {
        Message reply;

        reply.setType(Messagetype::DeleteAccountResponse);
        reply.setSequence(msg.sequence());
        reply.payload()["success"] = false;
        reply.payload()["reason"] = "invalid user session";

        se->send(reply);
        return;
    }
    int userId = userSession->userid();

    std::cout<< "[DeleteAccount] userid="<< userId<< std::endl;
    UserModel model;
    bool ok = model.deleteAccount(userId);
    Message reply;
    reply.setType(Messagetype::DeleteAccountResponse);
    reply.setSequence(msg.sequence());
    reply.setReceiverId(userId);

    if(!ok)
    {
        reply.payload()["success"] = false;
        reply.payload()["reason"] = "delete account failed";

        se->send(reply);

        return;
    }
    auto redis =RedisPool::instance().getConnection();
    if(redis)
    {
        if(!redis->setUserOffline(userId))
        {
            std::cout<< "[DeleteAccount] "<< "redis set offline failed"<< std::endl;
        }

        RedisPool::instance().releaseConnection(redis);
    }
    userSession->setAuthenticated(false);
    SessionManager::instance().unbindUser(userSession);
    std::cout<< "[DeleteAccount] "<< "unbind user "<< userId  << std::endl;
    reply.payload()["success"] = true;
    reply.payload()["message"] = "account deleted";
    se->send(reply);
}