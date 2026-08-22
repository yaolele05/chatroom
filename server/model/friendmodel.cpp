#include "friendmodel.h"
#include "../database/connectionpool/mysqlpool.h"
#include "../database/mysql/mysqlclient.h"
#include "../database/mysql/mysqlstatement.h"
#include <cstdint>
#include <chrono>
#include<iostream>
#include <cerrno>
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
    if(rela.userId() <= 0 || rela.friendId() <= 0)
    {
        std::cerr<<"invalid friend id"<<std::endl;
        return false;
    }
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
     if(!isFriend(userid, friendid))
    {
        return false;
    }
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
    auto stmt=conn->prepare(R"(SELECT id FROM user_friend WHERE (user_id=? AND friend_id=?) OR (user_id=? AND friend_id=?)  LIMIT 1)");
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
    auto stmt=conn->prepare(R"(SELECT id,user_id,friend_id,status,create_time From user_friend Where user_id=? OR friend_id=?)");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return friends;

    }

    stmt->bind(0,userid);
    stmt->bind(1,userid);
    auto result=stmt->query();
    while(result.fetch())
    {   
        auto rela=makeFriend(result);
        if(rela.userId()!=userid)
        {
            int tmp=rela.userId();
            rela.setUserId(rela.friendId());
            rela.setFriendId(tmp);
        }
        friends.push_back(rela);
    }
    MysqlPool::instance().releaseConnection(conn);
    return friends;
}
bool FriendModel::createFriendRequest(int fromUserId,int toUserId)
{

    auto conn = MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }

    auto stmt = conn->prepare(R"( SELECT id, status  FROM friend_request  WHERE from_user_id = ? AND to_user_id = ?  ORDER BY id DESC   LIMIT 1)");

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    stmt->bind(0, fromUserId);
    stmt->bind(1, toUserId);

    auto result = stmt->query();

    if(result.fetch())
    {
        int requestId = result.get<int>(0);
        int status = result.get<int>(1);

        if(status == 0)
        {
            MysqlPool::instance().releaseConnection(conn);
            return true;
        }
        //已经接受了
    
             //允许重新申请
        auto updateStmt = conn->prepare(R"( UPDATE friend_request  SET status = 0, create_time = CURRENT_TIMESTAMP WHERE id = ?)");
        if(!updateStmt)
        {
         MysqlPool::instance().releaseConnection(conn);
        return false;
        }
        updateStmt->bind(0, requestId);
        bool ok = updateStmt->execute();
        MysqlPool::instance().releaseConnection(conn);

        return ok;
    }
    // 第一次申请
    auto insertStmt = conn->prepare(R"( INSERT INTO friend_request ( from_user_id, to_user_id, status,  create_time )    VALUES (?, ?, 0, CURRENT_TIMESTAMP))");

    if(!insertStmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    insertStmt->bind(0, fromUserId);
    insertStmt->bind(1, toUserId);

    bool ok = insertStmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
std::vector<FriendModel::FriendRequest>FriendModel::findPendingFriendRequest(int userId)
{
    std::vector<FriendRequest> requests;
    auto conn = MysqlPool::instance().getConnection();
    if(!conn)
    {
        return requests;
    }
    auto stmt = conn->prepare(R"( SELECT  id,  from_user_id,  to_user_id,   status,
        UNIX_TIMESTAMP(create_time)    FROM friend_request
        WHERE to_user_id = ?  AND status = 0  ORDER BY create_time DESC)");

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return requests;
    }
    stmt->bind(0, userId);
    auto result = stmt->query();
    while(result.fetch())
    {
        FriendRequest request;
        request.id = result.get<int>(0);
        request.fromUserId = result.get<int>(1);
        request.toUserId = result.get<int>(2);
        request.status = result.get<int>(3);
        request.createTime = result.get<std::int64_t>(4);

        requests.push_back(request);
    }

    MysqlPool::instance().releaseConnection(conn);

    return requests;
}
std::optional<FriendModel::FriendRequest> FriendModel::findFriendRequest(int requestId)
{
    auto conn = MysqlPool::instance().getConnection();

    if(!conn)
    {
        return std::nullopt;
    }

    auto stmt = conn->prepare(R"( SELECT id,  from_user_id,  to_user_id,  status,     UNIX_TIMESTAMP(create_time)  FROM friend_request      WHERE id = ? )");

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return std::nullopt;
    }

    stmt->bind(0, requestId);

    auto result = stmt->query();

    std::optional<FriendRequest> request;

    if(result.fetch())
    {
        FriendRequest r;

        r.id = result.get<int>(0);
        r.fromUserId = result.get<int>(1);
        r.toUserId = result.get<int>(2);
        r.status = result.get<int>(3);
        r.createTime = result.get<std::int64_t>(4);

        request = r;
    }

    MysqlPool::instance().releaseConnection(conn);

    return request;
}
bool FriendModel::updatefRequestStatus(int userId,int friendId,int status)
{
    auto conn = MysqlPool::instance().getConnection();

    if(!conn)
    {
        return false;
    }

    auto stmt = conn->prepare(R"( UPDATE friend_request SET status = ? 
         WHERE status = 0
         AND(from_user_id=? AND to_user_id=?)
         OR (from_user_id=? AND to_user_id=?))");

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    stmt->bind(0,status);
    stmt->bind(1, userId);
    stmt->bind(2,friendId);
    stmt->bind(3,friendId);
    stmt->bind(4,userId);
    bool ok = stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}