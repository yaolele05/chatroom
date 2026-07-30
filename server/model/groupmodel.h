#pragma once
#include <vector>
#include <optional>
#include "entity/group.h"
#include "entity/groupmember.h"

class GroupModel
{
    public:
    bool createGroup(const Group& group);
    bool removeGroup(int groupid);
    bool updateGroup(const Group& group);
    std::optional<Group>findGroup(int groupid);
      std::vector<Group>queryGroups(int userid);
   bool addMember(const GroupMember& member);
   bool removeMember(int groupid,int userid );
   bool updateMemberRole(int groupid,int userid,GroupRole role);

    std::vector<GroupMember>queryMembers(int groupid);
};