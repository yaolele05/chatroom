#include "../database/mysql/mysqlresult.h"
#include "../database/mysql/mysqlclient.h"
#include "../database/mysql/mysqlstatement.h"
#include "../database/connectionpool/mysqlpool.h"
#include <cstdint>
#include <chrono>
#include "groupmodel.h"
#include "entity/group.h"
#include <stdexcept>
#include <iostream>
#include <cerrno>

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
    auto stmt=conn->prepare(R"(SELECT role From chatgroup_mamber WHERE group_id=? AND user_id=?)");
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