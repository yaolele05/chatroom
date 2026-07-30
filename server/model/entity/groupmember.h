#pragma once
#include <chrono>

enum class GroupRole
{
    Member=0,Admin=1,Owner=2
};
class GroupMember
{
    public:
    GroupMember()=default;

    int groupId() const;
    void setGroupId(int groupId);

    int userId() const;
    void setUserId(int userId);
    GroupRole role() const;
    void setRole(GroupRole role);
      std::chrono::system_clock::time_point JoinTime() const;
    void setJoinTime(const std::chrono::system_clock::time_point& time);

    private:
    int groupId_;
    int userId_;
    int role_;
    GroupRole role_{GroupRole::Member};
    std::chrono::system_clock::time_point joinTime_;
};