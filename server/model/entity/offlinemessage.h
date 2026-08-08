#pragma once
#include <cstdint>
#include <chrono>
enum class OfflineType
{
    ChatMessage = 1,
    File = 2,
    FriendRequest = 3,
    GroupRequest = 4
};
class OfflineMessage
{

    public:
    OfflineMessage()=default;
    std::int64_t id() const
    {
        return id_;
    }
    void setId(std::int64_t id)
    {
        id_=id;
    }
    int userId() const
    {
        return userId_;
    }
    void setUserId(int userId)
    {
        userId_=userId;
    }
    std::int64_t messageId() const
    {
        return messageId_;
    }
    void setMessageId(std::int64_t messageId)
    {
        messageId_=messageId;
    }
    std::chrono::system_clock::time_point createTime() const
    {
        return createTime_;
    }
    void setCreateTime(const std::chrono::system_clock::time_point& createTime)
    {
        createTime_=createTime;
    }
    OfflineType type() const
    {
        return type_;
    }

    void setType(OfflineType type)
    {
        type_=type;
    }

    private:
    std::int64_t id_;
    int userId_;
    std::int64_t messageId_;
    std::chrono::system_clock::time_point createTime_;
     OfflineType type_;
};