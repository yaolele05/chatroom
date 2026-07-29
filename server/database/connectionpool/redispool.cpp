#include "redispool.h"
#include "../redis/redisclient.h"
#include <iostream>

RedisPool::RedisPool()
{}
RedisPool& RedisPool::instance()
{
    static RedisPool pool;
    return pool;
}
RedisPool::~RedisPool()
{
    {
        std::lock_guard<std::mutex>lock(mutex_);
        running_=false;
        while(!connections_.empty())
        {
            connections_.pop();
        }
    }
    cv_.notify_all();
}
bool RedisPool::init(const std::string& host,uint16_t port,size_t size)
{
    running_=true;
    for(int i=0;i<size;i++)
    {
        auto client=std::make_shared<RedisClient>();

        if(!client->connect(host,port))
        {
            std::cerr<<"redis connect failed"<<std::endl;
            return false;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            connections_.push(client);
        }
    }

    maxSize_=size;
    return true;
}
std::shared_ptr <RedisClient> RedisPool::getConnection()
{
    std::unique_lock<std::mutex> lock(mutex_);

    cv_.wait(lock,[this]()
    {
    return !connections_.empty() ||!running_ ;
    });

    if(!running_)
    {
        return nullptr;
    }

    auto conn=connections_.front();
    connections_.pop();
    return conn;
}
void RedisPool::releaseConnection(std::shared_ptr<RedisClient> conn)
{
   if(!conn)
    return ;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(!running_)
        return ;
        connections_.push(conn);
    }
   
    cv_.notify_all();
}