#include "loginservice.h"
#include "../../model/usermodel.h"
#include "../../session/usersession.h"
#include "../../database/redis/redisclient.h"
#include "../../database/connectionpool/redispool.h"
#include "../../security/auth/token.h"
LoginService& LoginService::instance()
{
    static LoginService instance;

    return instance;
}
bool LoginService::login(const std::shared_ptr<UserSession>& session,const std::string& username,const std::string& password)
{

     UserModel userModel;
    auto user=userModel.findByName(username);
  
    if(!user.has_value())
    {
        return false;
    }
    
    if(user->passwordHash()!=password)
    {
        return false;
    }

    auto redis=RedisPool::instance().getConnection();
    if(!redis)
    {
        return false;
    }
    if(!redis->setUserOnline(user->id()))
    {
        RedisPool::instance().releaseConnection(redis);
        return false;
    }
    std::string token=Token::generate();
     if (!redis->saveToken(user->id(), token))
    {
        redis->setUserOffline(user->id());
    RedisPool::instance().releaseConnection(redis);
    session->logout();

    return false;
    }
    redis->refreshHeartbeat(user->id());
    RedisPool::instance().releaseConnection(redis);

    session->login(user->id(),user->username());
     return true;
}
bool LoginService::logout(const std::shared_ptr<UserSession>& session)
{
    if(!session ||!session->authenticated())
    {
        return false;
    }
    bool ok=false;
    auto redis=RedisPool::instance().getConnection();
    if(redis)
    {
     
     bool offline=redis->setUserOffline(session->userid());
     bool token=redis->deleteToken(session->userid());
     ok= offline && token;
     
     RedisPool::instance().releaseConnection(redis);
    }
   
    session->logout();
    return ok;
}

