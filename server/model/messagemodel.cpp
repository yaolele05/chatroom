#include "messagemodel.h"
#include "../database/connectionpool/mysqlpool.h"
#include "../database/mysql/mysqlclient.h"
#include "../database/mysql/mysqlstatement.h"
#include <chrono>
ChatMessage MessageModel::makeMessage(MysqlResult& result)
{
    ChatMessage message;
    message.setId(result.get<std::int64_t>(0));
    message.setSendId(result.get<int>(1));
    message.setReceiverId(result.get<int>(2));
    message.setGroupId(result.get<int>(3));
    message.setType(static_cast<MessageType>(result.get<int>(4)));
    message.setContent(result.get<std::string>(5));
    auto timestamp =result.get<std::int64_t>(6);
   message.setSendTime(std::chrono::system_clock::time_point(std::chrono::seconds(timestamp)));
    return message;
}
bool MessageModel::insert(ChatMessage& message)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
  bool ok=false;

  {
    auto stmt=conn->prepared(R"(INSERT INTO message(
        sender_id,receiver_id,group_id,type,content,create_time)VALUES(?,?,?,?,?,?)
    )");

     if(!stmt)
     {
    MysqlPool::instance().releaseConnection(conn);
    return false;
     }
    stmt->bind(0, message.sendId());
    stmt->bind(1, message.receiverId());
    stmt->bind(2, message.groupId());
    stmt->bind(3, static_cast<int>(message.type()));
    stmt->bind(4, message.content());
    stmt->bind(5, std::chrono::duration_cast<std::chrono::seconds>(message.sendTime().time_since_epoch()).count());

     ok=stmt->execute();

    if(ok)
    {
        message.setId(static_cast<std::int64_t>(conn->lastInsertId()));
   
       
    }
   }
    MysqlPool::instance().releaseConnection(conn);
    return ok;

}
std::optional<ChatMessage>MessageModel::findById(std::int64_t id)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return std::nullopt;
    }
  std::optional<ChatMessage> resultMessage;
   { auto stmt=conn->prepared("SELECT "   "id,"   "sender_id,"   "receiver_id,"
        "group_id,"   "type,"   "content,"      "create_time "
        "FROM message "   "WHERE id=?" );
    if(!stmt)
    {
    MysqlPool::instance().releaseConnection(conn);
    return std::nullopt;

    }
    stmt->bind(0,id);

    auto result=stmt->query();
    if(result.fetch())
    {
        resultMessage=makeMessage(result);
   
    }
  }
    MysqlPool::instance().releaseConnection(conn);
    return resultMessage;
}
std::vector<ChatMessage> MessageModel::findPriHistory(int userid, int peerid,size_t limit,size_t offset)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return {};
    }
     std::vector<ChatMessage> messages;
     {
    auto stmt=conn->prepared("SELECT "  "id,"  "sender_id,"  "receiver_id,"   "group_id,"
        "type,"    "content,"   "create_time "
        "FROM message "
        "WHERE (sender_id=? AND receiver_id=?) OR (sender_id=? AND receiver_id=?) "
        "ORDER BY create_time DESC "   "LIMIT ? OFFSET ?"
    );
    if(!stmt)
    {
    MysqlPool::instance().releaseConnection(conn);
    return {};

    }
    stmt->bind(0,userid);
    stmt->bind(1,peerid);
    stmt->bind(2,peerid);
    stmt->bind(3,userid);
    stmt->bind(4,static_cast<int>(limit));
    stmt->bind(5,static_cast<int>(offset));

    auto result=stmt->query();
    std::vector<ChatMessage> messages;
    while(result.fetch())
    {
        auto message=makeMessage(result);
        messages.push_back(message);
    }
  }
    MysqlPool::instance().releaseConnection(conn);
    return messages;
}
std::vector<ChatMessage> MessageModel::findGroupHistory(int groupid,size_t limit,size_t offset)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return {};
    }

    auto stmt=conn->prepare("SELECT "  "id,"  "sender_id,"   "receiver_id,"  "group_id,"  "type,"
       "content,"    "create_time "   "FROM message "     "WHERE group_id=? "  "ORDER BY create_time DESC "
        "LIMIT ? OFFSET ?");

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return {};
    }

    stmt->bind(0,groupid);
    stmt->bind(1,static_cast<int>(limit));
    stmt->bind(2,static_cast<int>(offset));

    auto result=stmt->query();
    std::vector<ChatMessage> messages;
    while(result.fetch())
    {
        auto message=makeMessage(result);
        messages.push_back(message);
    }
    MysqlPool::instance().releaseConnection(conn);
    return messages;
}
std::vector<ChatMessage> MessageModel::findUserGroupHistory(int userid,int groupid,size_t limit,size_t offset)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return {};
    }
     std::vector<ChatMessage> messages;

     {
    auto stmt=conn->prepare("SELECT "  "id,"   "sender_id,"  "receiver_id,"   "group_id,"
   "type,"   "content,"  "create_time "
  "FROM message "  "WHERE group_id=? AND sender_id=? "   "ORDER BY create_time DESC "   "LIMIT ? OFFSET ?");

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return {};
    }

    stmt->bind(0,groupid);
    stmt->bind(1,userid);
    stmt->bind(2,static_cast<int>(limit));
    stmt->bind(3,static_cast<int>(offset));

    auto result=stmt->query();
    while(result.fetch())
    {
        auto message=makeMessage(result);
        messages.push_back(message);
    }
   }
    MysqlPool::instance().releaseConnection(conn);
    return messages;
}