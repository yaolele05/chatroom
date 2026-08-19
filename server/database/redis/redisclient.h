#pragma once

#include <hiredis/hiredis.h>

#include <memory>
#include <optional>
#include <string>
#include<cstdarg>

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

     bool subscribe(const std::string& channel);
     bool unsubscribe(const std::string& channel);

     bool setUserOnline(int userid);
     bool setUserOffline(int userid);
    bool isUserOnline(int userid);

    bool saveToken(int userid, const std::string& token);
    std::optional<std::string> getToken(int userid);
    bool deleteToken(int userid);
     
    RedisResult command(const std::string& cmd);
    RedisResult command(const char* com,...);
    //RedisResult commandBinary(const char* com,...);


    bool publish(const std::string& channel,const std::string& message);

    bool refreshHeartbeat(int userid);

    bool blockFriend(uint32_t userId,uint32_t friendId);
     bool unblockFriend(uint32_t userId,uint32_t friendId);
     bool isBlocked(uint32_t userId,uint32_t friendId);

private:

    redisContext* context_{nullptr};
};