#include "redisresult.h"
RedisResult::RedisResult(redisReply* reply):reply_(reply)
{

}
RedisResult::~RedisResult()
{
    if(reply_)
    {
        freeReplyObject(reply_);

        reply_=nullptr;
    }
}
bool RedisResult::valid() const
{
    return reply_!=nullptr;
}
bool RedisResult::isString() const
{
    return valid()&&reply_->type==REDIS_REPLY_STRING;
}
bool RedisResult::isInteger() const
{
    return valid()&&reply_->type==REDIS_REPLY_INTEGER;
}
bool RedisResult::isArray() const
{
    return valid()&&reply_->type==REDIS_REPLY_ARRAY;
}
bool RedisResult::isStatus() const
{
    return valid()&&reply_->type==REDIS_REPLY_STATUS;
}
bool RedisResult::isNil() const
{
    return valid() &&reply_->type==REDIS_REPLY_NIL;
}
bool RedisResult::isError() const
{
    return valid()&&reply_->type==REDIS_REPLY_ERROR;
}
std::optional<std::string>RedisResult::string() const
{
    if(!isString())
     return std::nullopt;

    return std::string(reply_->str,reply_->len);
}
long long RedisResult::integer() const
{
    if(!isInteger())  
    return 0;

    return reply_->integer;
}
std::vector<std::string> RedisResult::array() const
{
    std::vector<std::string> result;

    if(!isArray())
    return result;

    for(size_t i=0;i<reply_->elements;++i)
    {

        redisReply* item =  reply_->element[i];
 
       if(item->type==REDIS_REPLY_STRING)
       {
         result.emplace_back( item->str,item->len);
       }
}

    return result;
}
std::string RedisResult::status() const
{
    if(!isStatus())
 return {};

    return std::string(reply_->str,reply_->len);
}
std::string RedisResult::error() const
{
    if(!isError())
     return {};


    return std::string(reply_->str, reply_->len);
}