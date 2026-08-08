#pragma once
#include <string>
#include <chrono>
#include <cstdint>
class Group
{
    public:
    Group()=default;
    std::int64_t id() const
    {
        return id_;
    }
     void setId(std::int64_t id)
    {
        id_=id;
    }
    const std::string& name() const
    {
        return name_;
    }
    void setName(const std::string& name)
    {
        name_=name;
    }
    
    int OwnerId() const
    {
        return ownerId_;
    }
    void setOwnerId(int ownerId)
    {
        ownerId_=ownerId;
    }
    const std::string& avatar() const
    {
        return avatar_;
    }
    void setAvatar(const std::string& avatar)
    {
        avatar_=avatar;
    }
    const std::string& description() const
    {
        return description_;
    }
    void setDescription(const std::string& description)
    {
        description_=description;
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
    std::int64_t id_;
    std::string name_;
    int ownerId_;
    std::string avatar_;
    std::string description_;
    std::chrono::system_clock::time_point createTime_;
};

