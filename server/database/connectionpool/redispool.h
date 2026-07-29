#pragma once
#include<string>
#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>
#include "../redis/redisclient.h"
class RedisPool
{
    public:
     static RedisPool& instance();
    
     bool init(const std::string& host,uint16_t port,size_t size);
     std::shared_ptr<RedisClient> getConnection();
    void releaseConnection(std::shared_ptr <RedisClient>);

    private:
    RedisPool();
    ~RedisPool();
    RedisPool(const RedisPool&)=delete;
    RedisPool& operator=(const RedisPool&)=delete;

    std::queue<std::shared_ptr<RedisClient>> connections_;
    std::mutex mutex_;
    std::condition_variable cv_;
    size_t maxSize_{0};
    bool running_{false};

};