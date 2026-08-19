#pragma once
#include <vector>
#include "entity/friend.h"
#include "../database/mysql/mysqlresult.h"
#include <optional>
class FriendModel
{
    public:
    struct FriendRequest
   {
    int id;
    int fromUserId;
    int toUserId;
    int status;
    std::int64_t createTime;
    };

    bool addFriend( Friend& rela);
    bool removeFriend(int userid,int friendid);
    bool isFriend(int userid,int friendid);
    
    std::vector<Friend> findFriends(int userid);
    ///std::vector<User> findFriendUsers(...);
    bool createFriendRequest(int fid,int tid);
    std::vector<FriendRequest> findPendingFriendRequest(int userid);
    std::optional<FriendRequest> findFriendRequest(int reid);
    bool updatefRequestStatus(int reid,int status);
    private:
    Friend makeFriend(MysqlResult& result);
    
};