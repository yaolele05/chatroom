#pragma once
#include <string>
#include <chrono>
#include<cstdint>
enum class MessageType
{
 
    Text,Image,File,Audio,Video,System
};
enum class MessageStatus
{
    Sending,Sent,Delivered,Read,Recalled
};
class Message
{
    public:
    Message()=default;

    std::int64_t id() const;
    void setId(std::int64_t id);
    int sendId() const;
    void setSendId(int sendId);
    int receiverId() const;
    void setReceiverId(int receiverId);

    int groupId() const;
    void setGroupId(int groupId);

    MessageType type() const;
    void setType(MessageType type);
    MessageStatus status() const;
    void setStatus(MessageStatus status);

    const std::string& content() const;
    void setContent(const std::string& content);

    std::chrono::system_clock::time_point sendTime() const;
    void setSendTime(const std::chrono::system_clock::time_point& time);

    private:
    std::int64_t id_{0};
    int sendId_{0};
    int receiverId_{0};
    int groupId_{0};
    MessageType type_{MessageType::Text};
    MessageStatus status_{MessageStatus::Sending};
    std::string content_;
    std::chrono::system_clock::time_point sendTime_;
};