#include "../database/mysql/mysqlresult.h"
#include "../database/mysql/mysqlclient.h"
#include "../database/mysql/mysqlstatement.h"
#include "../database/connectionpool/mysqlpool.h"
#include <cstdint>
#include <chrono>
#include "groupmodel.h"
#include "entity/group.h"
#include <stdexcept>////
#include <iostream>
#include <cerrno>
#include <nlohmann/json.hpp>
Group GroupModel::makeGroup(MysqlResult& result)
{
    Group group;
    group.setId(result.get<std::int64_t>(0));
    group.setName(result.get<std::string>(1));
    group.setOwnerId(result.get<int>(2));
    group.setAvatar(result.get<std::string>(3));
    group.setDescription(result.get<std::string>(4));
    auto timestamp=result.get<std::int64_t>(5);
    group.setCreateTime(std::chrono::system_clock::time_point(std::chrono::seconds(timestamp)));
    return group;
}
GroupMember GroupModel::makeGroupMember(MysqlResult& result)
{
    GroupMember member;
    member.setId(result.get<std::int64_t>(0));
    member.setGroupId(result.get<std::int64_t>(1));
    member.setUserId(result.get<std::int64_t>(2));
    member.setRole(static_cast<GroupRole>(result.get<int>(3)));
    auto timestamp=result.get<std::int64_t>(4);
    member.setCreateTime(std::chrono::system_clock::time_point(std::chrono::seconds(timestamp)));
    return member;
}
bool GroupModel::create(Group& group)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        std::cout<<"Failed to get connection from pool"<<std::endl;
        return false;
    }
    if(!conn->beginTransaction())
    {
        std::cerr<<"Failed to begin transaction: "<<conn->error()<<std::endl;
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    auto stmt=conn->prepare(R"(INSERT INTO chat_group(name,owner_id,avatar,description,create_time)VALUES(?,?,?,?,?))");
    if(!stmt)
    {
        std::cerr<<"Failed to insert group: "<<conn->error()<<std::endl;
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,group.name());
    stmt->bind(1,group.OwnerId());
    stmt->bind(2,group.avatar());
    stmt->bind(3,group.description());
    auto timestamp=std::chrono::duration_cast<std::chrono::seconds>(group.createTime().time_since_epoch()).count();
    stmt->bind(4,timestamp);

    if(!stmt->execute())
    {
        std::cerr<<"insert chatgroup failed "<<conn->error()<<std::endl;
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    group.setId(static_cast<int64_t>(conn->lastInsertId()));
    auto memberStmt=conn->prepare(R"(INSERT INTO chatgroup_member(group_id,user_id,role,create_time)VALUES(?,?,?,?))");
    if(!memberStmt)
    {

        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    memberStmt->bind(0,group.id());
    memberStmt->bind(1,group.OwnerId());
    memberStmt->bind(2,static_cast<int>(GroupRole::Owner));
    memberStmt->bind(3,timestamp);

    if(!memberStmt->execute())
    {
        std::cerr<<"Failed to insert chatgroup member: "<<conn->error()<<std::endl;
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    if(!conn->commit())
    {
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    MysqlPool::instance().releaseConnection(conn);
    return true;

}
std::optional<Group> GroupModel::findById(std::int64_t groupid)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return std::nullopt;
    }
    auto stmt=conn->prepare(R"(SELECT id,name,owner_id,avatar,description,create_time FROM chat_group WHERE id=?)");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return std::nullopt;
    }
    stmt->bind(0,groupid);
    auto result=stmt->query();
    if(result.fetch())
    {
        auto group=makeGroup(result);
        MysqlPool::instance().releaseConnection(conn);
        return group;
    }
    MysqlPool::instance().releaseConnection(conn);
    return std::nullopt;
}
bool GroupModel::addGroupMember( GroupMember& member)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
    auto stmt=conn->prepare(R"(INSERT INTO chatgroup_member(group_id,user_id,role,create_time)VALUES(?,?,?,?))");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,member.groupId());
    stmt->bind(1,member.userId());
    stmt->bind(2,static_cast<int>(member.role()));
    auto timestamp=std::chrono::duration_cast<std::chrono::seconds>(member.createTime().time_since_epoch()).count();
    stmt->bind(3,timestamp);

    if(!stmt->execute())
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    member.setId(static_cast<int64_t>(conn->lastInsertId()));
    MysqlPool::instance().releaseConnection(conn);
    return true;
}
bool GroupModel::removeGroupMember(std::int64_t groupid,int userid)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
    auto stmt=conn->prepare(R"(DELETE FROM chatgroup_member WHERE group_id=? AND user_id=?)");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,groupid);
    stmt->bind(1,userid);
    if(!stmt->execute())
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    MysqlPool::instance().releaseConnection(conn);
    return true;
}
bool GroupModel::removeGroup(int64_t groupId)
{
    auto conn=MysqlPool::instance().getConnection();

    if(!conn)
        return false;

    auto stmt=conn->prepare("DELETE FROM chat_group WHERE id=?");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,groupId);
    bool ok=stmt->execute();

    MysqlPool::instance().releaseConnection(conn);

    return ok;
}
std::vector<GroupMember>GroupModel::findGroupMembers(std::int64_t groupid)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return {};
    }
    auto stmt=conn->prepare(R"(SELECT id,group_id,user_id,role,create_time FROM chatgroup_member WHERE group_id=?)");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return {};
    }
    stmt->bind(0,groupid);
    auto result=stmt->query();
    std::vector<GroupMember> members;
    while(result.fetch())
    {
        auto member=makeGroupMember(result);
        members.push_back(member);
    }
    MysqlPool::instance().releaseConnection(conn);
    return members;
}
std::vector<Group> GroupModel::findUserGroups(int userid)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return {};
    }
    auto stmt=conn->prepare(R"(SELECT g.id,g.name,g.owner_id,g.avatar,g.description,g.create_time
        FROM chat_group g
        JOIN chatgroup_member m ON g.id=m.group_id
        WHERE m.user_id=?)");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return {};
    }
    stmt->bind(0,userid);
    auto result=stmt->query();
    std::vector<Group> groups;
    while(result.fetch())
    {
        auto group=makeGroup(result);
         std::cout << "findUserGroups:"<< " userid=" << userid<< " groupId=" << group.id()<< " name=" << group.name()<< " ownerId=" << group.OwnerId()<< std::endl;
        groups.push_back(group);
    }
       std::cout << "findUserGroups count="<< groups.size()<< std::endl;
    MysqlPool::instance().releaseConnection(conn);
    return groups;
}
bool GroupModel::isGroupMember(std::int64_t groupid,int userid)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
    auto stmt=conn->prepare(R"(SELECT id FROM chatgroup_member WHERE group_id=? AND user_id=?)");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,groupid);
    stmt->bind(1,userid);
    auto result=stmt->query();
    bool isMember=result.fetch();
    MysqlPool::instance().releaseConnection(conn);
    return isMember;
}
GroupRole GroupModel::getGroupMemberRole(std::int64_t groupid,int userid)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return GroupRole::Member;
    }
    auto stmt=conn->prepare(R"(SELECT role From chatgroup_member WHERE group_id=? AND user_id=?)");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return GroupRole::Member;
    }
    stmt->bind(0,groupid);
    stmt->bind(1,userid);
    auto result=stmt->query();
    if(result.fetch())
    {
        auto role=static_cast<GroupRole>(result.get<int>(0));
        MysqlPool::instance().releaseConnection(conn);
        return role;
    }
    MysqlPool::instance().releaseConnection(conn);
    return GroupRole::Member;
}
bool GroupModel::createJoinRequest(std::int64_t groupId,int userId)
{
    auto conn = MysqlPool::instance().getConnection();
    if(!conn)
    {
        std::cerr<< "[GroupModel] get mysql connection failed"<< std::endl;
        return false;
    }
    auto stmt = conn->prepare(R"( INSERT INTO group_join_request
        (group_id, from_user_id, status, create_time)
        VALUES (?, ?, 0, ?)
     )");

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0, groupId);
    stmt->bind(1, userId);
    auto timestamp =std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    stmt->bind(2, timestamp);
    bool ok = stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
bool GroupModel::hasPendingJoinRequest(std::int64_t groupId,int userId)
{
    auto conn = MysqlPool::instance().getConnection();
    if(!conn)
        return false;
    auto stmt = conn->prepare(R"( SELECT id  FROM group_join_request  WHERE group_id=?  AND from_user_id=?  AND status=0  LIMIT 1 )");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0, groupId);
    stmt->bind(1, userId);
    auto result = stmt->query();
    bool exists = result.fetch();
    MysqlPool::instance().releaseConnection(conn);

    return exists;
}
nlohmann::json GroupModel::findPendingJoinRequests( std::int64_t groupId)
{
    auto conn = MysqlPool::instance().getConnection();
    if(!conn)
    {
        std::cerr << "[GroupModel] get mysql connection failed"<< std::endl;
        return nlohmann::json::array();
    }

    auto stmt = conn->prepare(R"( SELECT  r.id,  r.group_id, r.from_user_id, r.create_time,
            u.username, u.nickname,  u.avatar,  u.signature
        FROM group_join_request r
        JOIN users u
            ON r.from_user_id = u.id
        WHERE r.group_id = ?
          AND r.status = 0
        ORDER BY r.create_time ASC  )");

    if(!stmt)
    {
        std::cerr<< "[GroupModel] prepare find pending join request failed: "  << conn->error()<< std::endl;
        MysqlPool::instance().releaseConnection(conn);
        return nlohmann::json::array();
    }

    stmt->bind(0, groupId);
    auto result = stmt->query();

    nlohmann::json requests =nlohmann::json::array();

    while(result.fetch())
    {
        nlohmann::json item;

        item["requestId"] =result.get<std::int64_t>(0);
        item["groupId"] =result.get<std::int64_t>(1);
        item["fromUserId"] = result.get<std::int64_t>(2);
        item["createTime"] =result.get<std::int64_t>(3);
        item["username"] =result.get<std::string>(4);
        item["nickname"] =  result.get<std::string>(5);
        item["avatar"] = result.get<std::string>(6);
        item["signature"] =result.get<std::string>(7);

        requests.push_back(item);
    }

    MysqlPool::instance().releaseConnection(conn);

    return requests;
}
bool GroupModel::acceptJoinRequest(std::int64_t requestId,int operatorId)
{
    auto conn = MysqlPool::instance().getConnection();

    if(!conn)
    {
        std::cerr<< "[GroupModel] get mysql connection failed" << std::endl;
        return false;
    }    
    if(!conn->beginTransaction())////一定是一个事务？？
    {
        std::cerr<< "[GroupModel] begin transaction failed: "<< conn->error()  << std::endl;
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    auto requestStmt = conn->prepare(R"( SELECT group_id,  from_user_id, status
      FROM group_join_request    WHERE id = ?  FOR UPDATE )");

    if(!requestStmt)
    {
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    requestStmt->bind(0, requestId);
    auto requestResult = requestStmt->query();
    if(!requestResult.fetch())
    {
        std::cerr<< "[GroupModel] join request not found, requestId="<< requestId<< std::endl;
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    std::int64_t groupId =requestResult.get<std::int64_t>(0);
    int fromUserId =requestResult.get<int>(1);
    int status =requestResult.get<int>(2);   
    if(status != 0)
    {
        std::cerr<< "[GroupModel] join request is not pending, requestId="<< requestId<< std::endl;
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    auto roleStmt = conn->prepare(R"( SELECT role  FROM chatgroup_member
        WHERE group_id = ? AND user_id = ? )");
    if(!roleStmt)
    {
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    roleStmt->bind(0, groupId);
    roleStmt->bind(1, operatorId);

    auto roleResult = roleStmt->query();

    if(!roleResult.fetch())
    {
        std::cerr << "[GroupModel] operator is not group member" << " groupId=" << groupId << " operatorId=" << operatorId            << std::endl;
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    GroupRole role = static_cast<GroupRole>( roleResult.get<int>(0));

    if(role != GroupRole::Owner &&role != GroupRole::Admin)
    {
        std::cerr<< "[GroupModel] operator has no permission"<< std::endl;
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);

        return false;
    }

    auto memberStmt = conn->prepare(R"(  SELECT id  FROM chatgroup_member
        WHERE group_id = ?  AND user_id = ? LIMIT 1 )");

    if(!memberStmt)
    {
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    memberStmt->bind(0, groupId);
    memberStmt->bind(1, fromUserId);

    auto memberResult = memberStmt->query();

    if(memberResult.fetch())
    {
        auto updateStmt = conn->prepare(R"( UPDATE group_join_request  SET status = 1  WHERE id = ?  AND status = 0 )");

        if(!updateStmt)
        {
            conn->rollback();
            MysqlPool::instance().releaseConnection(conn);
            return false;
        }

        updateStmt->bind(0, requestId);

        if(!updateStmt->execute())
        {
            conn->rollback();
            MysqlPool::instance().releaseConnection(conn);
            return false;
        }

        if(!conn->commit())
        {
            conn->rollback();
            MysqlPool::instance().releaseConnection(conn);
            return false;
        }

        MysqlPool::instance().releaseConnection(conn);

        return true;
    }

   
    auto insertStmt = conn->prepare(R"(  INSERT INTO chatgroup_member
        (group_id, user_id, role, create_time)
        VALUES (?, ?, ?, ?) )");

    if(!insertStmt)
    {
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    insertStmt->bind(0, groupId);
    insertStmt->bind(1, fromUserId);
    insertStmt->bind( 2,static_cast<int>(GroupRole::Member));

    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() .time_since_epoch()).count();

    insertStmt->bind(3, timestamp);

    if(!insertStmt->execute())
    {
        std::cerr << "[GroupModel] insert group member failed: "<< conn->error()<< std::endl;
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);

        return false;
    }

    auto updateStmt = conn->prepare(R"( UPDATE group_join_request SET status = 1
        WHERE id = ? AND status = 0 )");
    if(!updateStmt)
    {
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    updateStmt->bind(0, requestId);

    if(!updateStmt->execute())
    {
        std::cerr<< "[GroupModel] update join request failed: "  << conn->error()   << std::endl;
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }  
    if(!conn->commit())
    {
        std::cerr << "[GroupModel] commit accept join request failed: "   << conn->error()<< std::endl;
        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    MysqlPool::instance().releaseConnection(conn);

    return true;
}
bool GroupModel::rejectJoinRequest(std::int64_t requestId,int operatorId)
{
    auto conn = MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
    auto requestStmt = conn->prepare(R"(
        SELECT  group_id,  status
        FROM group_join_request
        WHERE id = ?
    )");

    if(!requestStmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    requestStmt->bind(0, requestId);
    auto requestResult =requestStmt->query();

    if(!requestResult.fetch())
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    std::int64_t groupId =requestResult.get<std::int64_t>(0);
    int status =requestResult.get<int>(1);

    if(status != 0)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    auto roleStmt = conn->prepare(R"(
        SELECT role
        FROM chatgroup_member
        WHERE group_id = ?
          AND user_id = ?
    )");

    if(!roleStmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    roleStmt->bind(0, groupId);
    roleStmt->bind(1, operatorId);
    auto roleResult =roleStmt->query();

    if(!roleResult.fetch())
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    GroupRole role =static_cast<GroupRole>(roleResult.get<int>(0));
    if(role != GroupRole::Owner &&role != GroupRole::Admin)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    auto updateStmt = conn->prepare(R"( UPDATE group_join_request
        SET status = 2  WHERE id = ?  AND status = 0
    )");

    if(!updateStmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    updateStmt->bind(0, requestId);
    bool ok =updateStmt->execute();
    MysqlPool::instance().releaseConnection(conn);

    return ok;
}
bool GroupModel::setGroupMemberRole(std::int64_t groupid,int userid, GroupRole role)
{
    auto conn = MysqlPool::instance().getConnection();

    if(!conn)
    {
        std::cerr<< "[GroupModel] get mysql connection failed"<< std::endl;
        return false;
    }
    if(role == GroupRole::Owner)
    {
        std::cerr<< "[GroupModel] cannot set member role to Owner"  << std::endl;

        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

   
    auto checkStmt = conn->prepare(R"( SELECT role  FROM chatgroup_member   WHERE group_id = ?     AND user_id = ?   LIMIT 1    )");

    if(!checkStmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    checkStmt->bind(0, groupid);
    checkStmt->bind(1, userid);

    auto result = checkStmt->query();

    if(!result.fetch())
    {
        std::cerr<< "[GroupModel] user is not group member"<< " groupId=" << groupid<< " userId=" << userid<< std::endl;

        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    GroupRole currentRole =static_cast<GroupRole>( result.get<int>(0));

    if(currentRole == GroupRole::Owner)
    {
        std::cerr<< "[GroupModel] cannot modify owner role"<< std::endl;

        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    auto updateStmt = conn->prepare(R"(
        UPDATE chatgroup_member
        SET role = ?
        WHERE group_id = ?
          AND user_id = ?
    )");
    if(!updateStmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    updateStmt->bind(0,static_cast<int>(role));

    updateStmt->bind(1, groupid);
    updateStmt->bind(2, userid);

    bool ok = updateStmt->execute();

    if(!ok)
    {
        std::cerr<< "[GroupModel] update group member role failed: "<< conn->error()<< std::endl;
    }

    MysqlPool::instance().releaseConnection(conn);

    return ok;
}