#pragma once

#include <string>
#include <chrono>
#include <cstdint>

enum class MessageType
{
    Text,
    Image,
    File,
    Audio,
    Video,
    System
};
/*enum class MessageStatus
{
    Sending,
    Sent,
    Delivered,
    Read,
    Recalled
};*/
class ChatMessage
{
public:
    ChatMessage() = default;
    std::int64_t id() const
    {
        return id_;
    }
    void setId(std::int64_t id)
    {
        id_ = id;
    }
    int sendId() const
    {
        return sendId_;
    }
    void setSendId(int sendId)
    {
        sendId_ = sendId;
    }
    int receiverId() const
    {
        return receiverId_;
    }
    void setReceiverId(int receiverId)
    {
        receiverId_ = receiverId;
    }
    int groupId() const
    {
        return groupId_;
    }
    void setGroupId(int groupId)
    {
        groupId_ = groupId;
    }
    MessageType type() const
    {
        return type_;
    }
    void setType(MessageType type)
    {
        type_ = type;
    }
    const std::string& content() const
    {
        return content_;
    }
    void setContent(const std::string& content)
    {
        content_ = content;
    }
    std::chrono::system_clock::time_point sendTime() const
    {
        return sendTime_;
    }
    void setSendTime(const std::chrono::system_clock::time_point& time)
    {
        sendTime_ = time;
    }
private:
    std::int64_t id_{0};
    int sendId_{0};
    int receiverId_{0};
    int groupId_{0};
    MessageType type_{MessageType::Text};
    // MessageStatus status_{MessageStatus::Sending};
    std::string content_;
    std::chrono::system_clock::time_point sendTime_;
};