#include "redisclient.h"
#include <iostream>
#include "redisresult.h"
#include <cstdarg>
RedisClient::RedisClient()
{
     
}
RedisClient::~RedisClient()
{
   disconnect();
}
bool RedisClient::connect(const std::string& host,uint16_t port)
{

    if(context_)
    {
        disconnect();
    }
    context_=redisConnect(host.c_str(),port);
    if(context_==nullptr)
    {
    std::cerr<<"redis connect create failed"<<std::endl;
    return false;
    }

    if(context_->err)
    {
        std::cerr<<"redis connect failed:"<<context_->errstr<<std::endl;
        redisFree(context_);
    context_=nullptr;
    return false;
    }
    
   return true;
}
void RedisClient::disconnect()
{
   
    if(context_)
    {
        redisFree(context_);
        context_=nullptr;
    }
}
bool RedisClient:: connected() const
{

     return context_ !=nullptr&& context_->err==0;

}
bool RedisClient::set(const std::string& key,const std::string& value)
{

    if(!connected())
    return false;

    redisReply*reply=static_cast<redisReply*>(redisCommand(context_,"SET %s %s",key.c_str(),value.c_str()));

    if(reply==nullptr)
    return false;
    bool success  =reply->type==REDIS_REPLY_STATUS && reply->str &&std::string(reply->str)=="OK";
    freeReplyObject(reply);

    return success;
}
bool RedisClient::setex(const std::string& key,int seconds,const std::string& value )
{

    if(!connected())
    return false;

    redisReply* reply=static_cast<redisReply*>(redisCommand(context_,"SETEX %s %d %s",key.c_str(),seconds,value.c_str()));
    if(reply==nullptr)
    return false;

    bool success=reply->type==REDIS_REPLY_STATUS &&std::string(reply->str)=="OK";

    freeReplyObject(reply);

    return success;
}
std::optional<std::string> RedisClient::get(const std::string& key)
{


    if(!connected())
    return std::nullopt;

    redisReply* reply=static_cast<redisReply*> (redisCommand(context_,"GET %s",key.c_str()));

    if(reply==nullptr)
    return std::nullopt;
    if(reply->type !=REDIS_REPLY_STRING)
    {
        freeReplyObject(reply);
        return std::nullopt;

    }
    std::string value(reply->str,reply->len);

    freeReplyObject(reply);
    return  value;


}
bool RedisClient::del(const std::string& key)
{

    if(!connected())
        return false;
    redisReply* reply=static_cast<redisReply*>(redisCommand(context_,"DEL %s",key.c_str()));

    if(reply==nullptr)
    return false;

    bool success= reply->type==REDIS_REPLY_INTEGER && reply->integer>0;
    freeReplyObject(reply);

    return success;

}
bool RedisClient::exists(const std::string& key)
{
  if(!connected())
  return false;
   redisReply* reply=static_cast<redisReply*>(redisCommand(context_,"EXISTS %s",key.c_str()));
   if(reply==nullptr)
   return false;
   bool result=reply->type==REDIS_REPLY_INTEGER&& reply->integer>0;
   freeReplyObject(reply);

   return result;
  
}
bool RedisClient::setUserOnline(int userid)
{
    return set("user:online:"+std::to_string(userid),"1");

}
bool RedisClient::setUserOffline(int userid)
{
    return del("user:online:"+std::to_string(userid));

}
bool RedisClient::isUserOnline(int userid)
{
    auto value=get("user:online:"+std::to_string(userid));

    return value.has_value();
}
bool RedisClient::blockFriend(uint32_t userId,uint32_t friendId)
{
     
   bool re= set("block:"+std::to_string(userId)+":"+std::to_string(friendId),"1");
   return re;
}
bool RedisClient::unblockFriend(uint32_t userId,uint32_t friendId)
{
    bool re=del("block:"+std::to_string(userId)+":"+std::to_string(friendId));
    return re;
}
bool RedisClient::isBlocked(uint32_t userId,uint32_t friendId)
{
    bool re=exists("block:"+std::to_string(userId)+":"+std::to_string(friendId));
    return re;
}