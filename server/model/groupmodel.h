#pragma once
#include <string>
#include <chrono>
#include <cstdint>
#include "entity/groupmember.h"
#include "entity/group.h"
#include "../database/mysql/mysqlresult.h"
#include <optional>
#include <vector>

class GroupModel
{
    public:
    bool create(Group& group);
   
    std::optional<Group> findById(std::int64_t groupid);
    bool addGroupMember(GroupMember& member);
    bool removeGroupMember(std::int64_t groupid,int userid);
    std::vector<GroupMember> findGroupMembers(std::int64_t groupid); 
    std::vector<Group> findUserGroups(int userid);
    bool isGroupMember(std::int64_t groupid,int userid);
    GroupRole getGroupMemberRole(std::int64_t groupid,int userid);
    bool removeGroup(int64_t groupId);
    private:
    Group makeGroup(MysqlResult& result);
    GroupMember makeGroupMember(MysqlResult& result);

};