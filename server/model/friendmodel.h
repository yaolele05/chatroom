#pragma once
#include <vector>
#include "entity/friend.h"
class FriendModel
{
    public:
    bool insert(const Friend& rela);
    bool removeFriend(int userid,int friendid);
    bool isFriend(int userid,int friendid);

    std::vector<Friend> queryFriendIds(int userid);
};