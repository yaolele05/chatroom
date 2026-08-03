#pragma once
#include <vector>
#include "entity/friend.h"
#include "../database/mysql/mysqlresult.h"
class FriendModel
{
    public:
    bool addFriend( Friend& rela);
    bool removeFriend(int userid,int friendid);
    bool isFriend(int userid,int friendid);
    
    std::vector<Friend> findFriends(int userid);

    private:
    Friend makeFriend(MysqlResult& result);
    
};