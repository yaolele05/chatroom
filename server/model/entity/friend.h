#pragma once
#include <string>
#include <chrono>
#include <cstdint>

class Friend
{
    public:
     Friend()=default;
     std::int64_t id() const 
     {
        return id_;
     }
     void setId(std::int64_t id)
     {
        id_=id;
     }
        std::int64_t userId() const
        {
            
            return userId_;
        }
    void setUserId(int userId)
    {
        userId_=userId;
    }
    int friendId() const
    {
        return friendId_;
    }
    void setFriendId(int friendId)
    {
        friendId_=friendId;
    }
    int status() const
    {
        return status_;
    }
    void setStatus(int status)
    {
        status_=status; 
    }
    std::chrono::system_clock::time_point createTime() const
    {
        return createTime_;
    }
    void setCreateTime(const std::chrono::system_clock::time_point& createTime)
    {
        createTime_=createTime;
    }

    private:
    std::int64_t id_{0};
    int userId_{0};
    int friendId_{0};
    int status_{1};
    std::chrono::system_clock::time_point createTime_{std::chrono::system_clock::now()};

};