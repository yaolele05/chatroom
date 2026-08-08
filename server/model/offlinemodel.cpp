#include "offlinemodel.h"
#include "../database/connectionpool/mysqlpool.h"
#include "../database/mysql/mysqlclient.h"
#include <vector>
#include <cstdint>
#include <chrono>
OfflineMessage OfflineMessageModel::makeOfflineMessage(const MysqlResult& result)
{
    OfflineMessage message;
    message.setId(result.get<std::int64_t>(0));
    message.setUserId(result.get<int>(1));
    message.setMessageId(result.get<std::int64_t>(2));
    message.setType(static_cast<OfflineType>(result.get<int>(3)));
    auto timestamp=result.get<std::int64_t>(4);
    message.setCreateTime(std::chrono::system_clock::time_point(std::chrono::seconds(timestamp)));
    return message;
}
bool OfflineMessageModel::insert(OfflineMessage& message)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
    auto stmt=conn->prepare(R"(INSERT INTO offline_message(user_id,message_id,type,create_time)VALUES(?,?,?,?))");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,message.userId());
    stmt->bind(1,message.messageId());
    stmt->bind(2,static_cast<int>(message.type()));
    auto timestamp=std::chrono::duration_cast<std::chrono::seconds>(message.createTime().time_since_epoch()).count();
    stmt->bind(3,timestamp);
    bool ok=stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
std::vector<OfflineMessage> OfflineMessageModel::findByUserId(int userid)
{
    std::vector<OfflineMessage> messages;
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return messages;
    }
    auto stmt=conn->prepare("SELECT id,user_id,message_id,type,create_time FROM offline_message WHERE user_id=?");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return messages;
    }
    stmt->bind(0,userid);
    auto result=stmt->query();
    while(result.fetch())
    {
        auto message=makeOfflineMessage(result);
        messages.push_back(message);
    }
    MysqlPool::instance().releaseConnection(conn);
    return messages;
}
bool OfflineMessageModel::remove(std::int64_t id)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
    auto stmt=conn->prepare("DELETE FROM offline_message WHERE id=?");
    if(!stmt)   
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,id);   
    bool ok=stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
bool OfflineMessageModel::clearUserMessages(int userid)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
    auto stmt=conn->prepare("DELETE FROM offline_message WHERE user_id=?");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,userid);
    bool ok=stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
