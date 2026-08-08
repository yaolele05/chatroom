#pragma once
#include <chrono>
#include <cstdint>

enum class GroupRole
{
   Member=0,
    Admin=1,
    Owner=2
};
class GroupMember
{
    public:
    GroupMember()=default;
    std::int64_t id() const
    {
        return id_;
    }
    void setId(std::int64_t id)
    {
        id_=id;
    }
    int groupId() const
    {
        return groupId_;
    }
    void setGroupId(std::int64_t groupId)
    {
        groupId_=groupId;
    }
    int userId() const
    {
        return userId_;
    }
    void setUserId(int userId)
    {
        userId_=userId;
    }
    GroupRole role() const
    {
        return role_;
    }
    void setRole(GroupRole role)
    {
        role_=role;
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
    std::int64_t groupId_;
    int userId_;
    GroupRole role_;
    std::chrono::system_clock::time_point createTime_;  

};