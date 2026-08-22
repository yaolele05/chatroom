#include "offlinemodel.h"
#include "../database/connectionpool/mysqlpool.h"
#include "../database/mysql/mysqlclient.h"
#include <vector>
#include <cstdint>
#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>

OfflineMessage OfflineMessageModel::makeOfflineMessage(const MysqlResult &result)
{
    OfflineMessage message;
    message.setId(result.get<std::int64_t>(0));
    message.setUserId(result.get<int>(1));
    message.setMessageId(result.get<std::int64_t>(2));
    message.setType(static_cast<OfflineType>(result.get<int>(3)));
    auto timestamp = result.get<std::int64_t>(4);
    message.setCreateTime(std::chrono::system_clock::time_point(std::chrono::seconds(timestamp)));
    return message;
}

bool OfflineMessageModel::insert(OfflineMessage &message)
{
    auto conn = MysqlPool::instance().getConnection();
    if (!conn)
    {
        return false;
    }
    auto stmt = conn->prepared(R"(INSERT INTO offline_message(user_id,message_id,type,create_time)VALUES(?,?,?,?))");
    if (!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0, message.userId());
    stmt->bind(1, message.messageId());
    stmt->bind(2, static_cast<int>(message.type()));
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(message.createTime().time_since_epoch()).count();
    stmt->bind(3, timestamp);
    bool ok = stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
std::vector<OfflineMessage> OfflineMessageModel::findByUserId(int userid)
{
    std::vector<OfflineMessage> messages;
    auto conn = MysqlPool::instance().getConnection();
    if (!conn)
    {
        return messages;
    }
    auto stmt = conn->prepared(
        "SELECT id,  user_id,  message_id,   type,   create_time "
        "FROM offline_message "
        "WHERE user_id=? "
        "ORDER BY create_time ASC");

    if (!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return messages;
    }
    stmt->bind(0, userid);
    auto result = stmt->query();
    while (result.fetch())
    {
        auto message = makeOfflineMessage(result);
        messages.push_back(message);
    }
    MysqlPool::instance().releaseConnection(conn);
    return messages;
}
bool OfflineMessageModel::remove(std::int64_t id)
{
    auto conn = MysqlPool::instance().getConnection();
    if (!conn)
    {
        return false;
    }
    auto stmt = conn->prepared("DELETE FROM offline_message WHERE id=?");
    if (!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0, id);
    bool ok = stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
bool OfflineMessageModel::clearUserMessages(int userid)
{
    auto conn = MysqlPool::instance().getConnection();
    if (!conn)
    {
        return false;
    }
    auto stmt = conn->prepared("DELETE FROM offline_message WHERE user_id=?");
    if (!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0, userid);
    bool ok = stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
int OfflineMessageModel::countPrivateUnread(int userId, int friendId)
{
    auto conn = MysqlPool::instance().getConnection();

    if (!conn)
    {
        return 0;
    }

    auto stmt = conn->prepared(R"(  SELECT COUNT(*)   FROM offline_message om  INNER JOIN message m
         ON om.message_id = m.id  WHERE om.user_id = ?  AND om.type = ?  AND m.sender_id = ?  AND m.receiver_id = ?   AND m.group_id = 0 )");
    if (!stmt)
    {
        std::cout << "[countPrivateUnread] prepare failed" << std::endl;
        MysqlPool::instance().releaseConnection(conn);
        return 0;
    }
    stmt->bind(0, userId);
    stmt->bind(1, static_cast<int>(OfflineType::ChatMessage));
    stmt->bind(2, friendId);
    stmt->bind(3, userId);

    auto result = stmt->query();
    int count = 0;
    if (result.fetch())
    {
        auto rawCount = result.get<int64_t>(0);
        count = static_cast<int>(rawCount);
    }

    MysqlPool::instance().releaseConnection(conn);
    return count;
}
bool OfflineMessageModel::clearPrivateMessages(int userId, int friendId)
{
    auto conn = MysqlPool::instance().getConnection();

    if (!conn)
    {
        return false;
    }

    auto stmt = conn->prepared(R"( DELETE FROM offline_message  WHERE user_id = ?  AND type = ? AND message_id IN
     (   SELECT id   FROM message  WHERE sender_id = ?  AND receiver_id = ?  AND group_id = 0  ) )");

    if (!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    stmt->bind(0, userId);
    stmt->bind(1, static_cast<int>(OfflineType::ChatMessage));
    stmt->bind(2, friendId);
    stmt->bind(3, userId);
    bool ok = stmt->execute();

    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
std::vector<OfflineMessage> OfflineMessageModel::findPrivateMessages(int userId, int friendId)
{
    std::vector<OfflineMessage> messages;
    auto conn = MysqlPool::instance().getConnection();

    if (!conn)
    {
        return messages;
    }

    auto stmt = conn->prepared(R"(  SELECT om.id,  om.user_id,   om.message_id,  om.type, om.create_time
    FROM offline_message om   INNER JOIN message m    ON om.message_id = m.id
   WHERE om.user_id = ?
   AND om.type = ?   AND m.sender_id = ?  AND m.receiver_id = ?    AND m.group_id = 0   ORDER BY om.create_time ASC )");
    if (!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return messages;
    }

    stmt->bind(0, userId);
    stmt->bind(1, static_cast<int>(OfflineType::ChatMessage));
    stmt->bind(2, friendId);
    stmt->bind(3, userId);
    auto result = stmt->query();
    while (result.fetch())
    {
        messages.push_back(makeOfflineMessage(result));
    }
    MysqlPool::instance().releaseConnection(conn);
    return messages;
}
nlohmann::json OfflineMessageModel::findGroupUnreadCounts(int userId)
{
    auto conn = MysqlPool::instance().getConnection();

    if (!conn)
        return nlohmann::json::array();

    auto stmt = conn->prepared(R"(
        SELECT m.group_id,  g.name,
            COUNT(*) AS unread_count
        FROM offline_message om
        INNER JOIN message m   ON om.message_id = m.id
        INNER JOIN chat_group g   ON m.group_id = g.id
        WHERE om.user_id = ?   AND om.type = ? AND m.group_id != 0
      GROUP BY m.group_id, g.name  ORDER BY m.group_id
    )");

    if (!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return nlohmann::json::array();
    }
    stmt->bind(0, userId);
    stmt->bind(1, static_cast<int>(OfflineType::ChatMessage));
    auto result = stmt->query();
    nlohmann::json groups = nlohmann::json::array();
    while (result.fetch())
    {
        nlohmann::json item;
        item["groupId"] = result.get<std::int64_t>(0);
        item["groupName"] = result.get<std::string>(1);
        item["unreadCount"] = result.get<int64_t>(2);
        groups.push_back(item);
    }
    MysqlPool::instance().releaseConnection(conn);

    return groups;
}
bool OfflineMessageModel::clearGroupMessages(int userId, int64_t groupId)
{
    auto conn = MysqlPool::instance().getConnection();
    if (!conn)
        return false;

    auto stmt = conn->prepared(R"(
        DELETE FROM offline_message
        WHERE user_id = ?  AND type = ?
          AND message_id IN
          (
              SELECT id  FROM message  WHERE group_id = ?
          ))");

    if (!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0, userId);
    stmt->bind(1, static_cast<int>(OfflineType::ChatMessage));
    stmt->bind(2, groupId);
    bool ok = stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
std::vector<OfflineMessage> OfflineMessageModel::findGroupMessages(int userId, int64_t groupId)
{
    std::vector<OfflineMessage> messages;
    auto conn = MysqlPool::instance().getConnection();
    if (!conn)
    {
        return messages;
    }
    auto stmt = conn->prepared(R"(
        SELECT  om.id,  om.user_id,
         om.message_id,  om.type, om.create_time
        FROM offline_message om  INNER JOIN message m
            ON om.message_id = m.id
        WHERE om.user_id = ? AND om.type = ?  AND m.group_id = ?
        ORDER BY om.create_time ASC )");

    if (!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return messages;
    }

    stmt->bind(0, userId);
    stmt->bind(1, static_cast<int>(OfflineType::ChatMessage));
    stmt->bind(2, groupId);

    auto result = stmt->query();

    while (result.fetch())
    {
        messages.push_back(makeOfflineMessage(result));
    }

    MysqlPool::instance().releaseConnection(conn);

    return messages;
}