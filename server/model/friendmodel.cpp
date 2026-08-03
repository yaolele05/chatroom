#include "friendmodel.h"
#include "../database/connectionpool/mysqlpool.h"
#include "../database/mysql/mysqlclient.h"
#include "../database/mysql/mysqlstatement.h"
#include <cstdint>
#include <chrono>

Friend FriendModel::makeFriend(MysqlResult& result)
{
    Friend rela;
    rela.setId(result.get<std::int64_t>(0));
    rela.setUserId(result.get<int>(1));
    rela.setFriendId(result.get<int>(2));
    rela.setStatus(result.get<int>(3));
    auto timestamp=result.get<std::int64_t>(4);
    rela.setCreateTime(std::chrono::system_clock::time_point(std::chrono::seconds(timestamp)));

    return rela;
}
bool FriendModel::addFriend( Friend& rela)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
    auto  stmt=conn->prepare(R"(INSERT INTO user_friend(
        user_id,friend_id,status,create_time)VALUES(?,?,?,?))");

        if(!stmt)
        {
            MysqlPool::instance().releaseConnection(conn);
            return false;
        }

        auto stamp=std::chrono::duration_cast<std::chrono::seconds>(rela.createTime().time_since_epoch()).count();
    stmt->bind(0,rela.userId());
    stmt->bind(1,rela.friendId());
    stmt->bind(2,rela.status());
    stmt->bind(3,stamp);

    if(!stmt->execute())
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    rela.setId(static_cast<std::int64_t>(conn->lastInsertId()));
    MysqlPool::instance().releaseConnection(conn);
    return true;
}
bool FriendModel::removeFriend(int userid,int friendid)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;

    }
    auto stmt=conn->prepare(R"(DELETE From user_friend WHERE (user_id=? AND friend_id=?)
        OR(user_id=? AND friend_id=?))");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,userid);
    stmt->bind(1,friendid);
    stmt->bind(2,friendid);
    stmt->bind(3,userid);
    if(!stmt->execute())    
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    MysqlPool::instance().releaseConnection(conn);
    return true;
}
bool FriendModel::isFriend(int userid,int friendid)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
    auto stmt=conn->prepare(R"(SELECT id FROM user_friend WHERE (user_id=? AND friend_id=?) OR (user_id=? AND friend_id=?))");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,userid);
    stmt->bind(1,friendid);
    stmt->bind(2,friendid);
    stmt->bind(3,userid);
    auto result=stmt->query();
    bool isfriend=result.fetch();
    MysqlPool::instance().releaseConnection(conn);

    return isfriend;
}
std::vector<Friend> FriendModel::findFriends(int userid)
{
    std::vector<Friend> friends;
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return friends;
    }
    auto stmt=conn->prepare(R"(SELECT id,user_id,friend_id,status,create_time From user_friend Where user_id=?)");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return friends;

    }

    stmt->bind(0,userid);
    auto result=stmt->query();
    while(result.fetch())
    {   
        auto rela=makeFriend(result);
        friends.push_back(rela);
    }
    MysqlPool::instance().releaseConnection(conn);
    return friends;
}
