#pragma once

#include <hiredis/hiredis.h>

#include <memory>
#include <optional>
#include <string>

class RedisResult;

class RedisClient
{
public:
    RedisClient();
    ~RedisClient();

    RedisClient(const RedisClient&) = delete;
    RedisClient& operator=(const RedisClient&) = delete;
    
    RedisClient(RedisClient&& other) noexcept;
     RedisClient& operator=(RedisClient&& other) noexcept;

public:

    bool connect(const std::string& host, uint16_t port);

    void disconnect();

    bool connected() const;

   

    bool set(const std::string& key,const std::string& value);

    bool setex(const std::string& key,int seconds,const std::string& value);

    std::optional<std::string> get(const std::string& key);

    bool del(const std::string& key);

    bool exists(const std::string& key);

    bool expire(const std::string& key,int seconds);

    
private:

    redisContext* context_{nullptr};
};