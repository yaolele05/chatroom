#pragma once

#include <hiredis/hiredis.h>

#include <optional>
#include <string>
#include <vector>


class RedisResult
{
public:

    explicit RedisResult(redisReply* reply);
    ~RedisResult();
    RedisResult(const RedisResult&) = delete;
    RedisResult& operator=(const RedisResult&) = delete;
public:


    bool valid() const;
    bool isString() const;
    bool isInteger() const;
    bool isArray() const;
    bool isStatus() const;
    bool isNil() const;
    bool isError() const;
    std::optional<std::string> string() const;


    long long integer() const;
    std::vector<std::string> array() const;
    std::string status() const;
    std::string error() const;

private:

    redisReply* reply_{nullptr};

};