#include "filereceivermodel.h"
#include "../database/connectionpool/mysqlpool.h"
#include "../database/mysql/mysqlclient.h"
#include "../database/mysql/mysqlstatement.h"
#include "../database/mysql/mysqlresult.h"
#include <cerrno>
#include <iostream>

FileReceiver FileReceiverModel::makeFileReceiver(MysqlResult& result)
{

    FileReceiver receiver;
    receiver.setId(result.get<int64_t>(0));
    receiver.setFileId(result.get<int64_t>(1));
    receiver.setUserId(result.get<int32_t>(2));
    receiver.setStatus(result.get<int32_t>(3));
    auto timestamp =result.get<int64_t>(4);
    receiver.setCreateTime(std::chrono::system_clock::time_point( std::chrono::seconds(timestamp)));
    return receiver;
}
bool FileReceiverModel::insert(FileReceiver& receiver)
{

     auto conn =MysqlPool::instance().getConnection();

     if(!conn)
    return false;
    auto stmt =conn->prepare(R"(INSERT INTO file_receiver(file_id, user_id,status)  VALUES(?,?,?))");

     if(!stmt)
   {
    MysqlPool::instance().releaseConnection(conn);

    return false;
   }
    stmt->bind(0,receiver.fileId());
    stmt->bind(1,receiver.userId());
    stmt->bind(2,receiver.status());
      bool ok =stmt->execute();
       if(ok)
    {
    receiver.setId( conn->lastInsertId());
    }

        MysqlPool::instance().releaseConnection(conn);
    return ok;

}
std::vector<FileReceiver>FileReceiverModel::findWaitingFiles(int userid)
{
     std::vector<FileReceiver> list;
     auto conn=MysqlPool::instance().getConnection();
     if(!conn)
     {
        return list;
     }
     auto stmt=conn->prepare(R"(SELECT  id,  file_id,  user_id,  status,  UNIX_TIMESTAMP(create_time)
        FROM file_receiver
        WHERE user_id=?
        AND status=0)");
        if(!stmt)
        {
            MysqlPool::instance().releaseConnection(conn);
            return list;

        }
        stmt->bind(0,userid);
        auto result=stmt->query();
        while(result.fetch())
        {
            list.push_back(makeFileReceiver(result));
        }
        MysqlPool::instance().releaseConnection(conn);
        return list;
}
bool FileReceiverModel::updateStatus(int64_t fileId,int userid,int status)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    return false;

    auto stmt=conn->prepare(  R"(UPDATE file_receiver SET status=? WHERE file_id=?  ANd user_id=?)");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,status);
    stmt->bind(1,fileId);
    stmt->bind(2,userid);

    bool ok=stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;

}
std::vector<FileReceiver>FileReceiverModel::findByFileId(int64_t fileId)
{
    std::vector<FileReceiver> receivers;
    auto conn = MysqlPool::instance().getConnection();
    if(!conn)
    {
        return receivers;
    }

    auto stmt = conn->prepare(  R"(SELECT id,  file_id, user_id, status,  UNIX_TIMESTAMP(create_time)   FROM file_receiver   WHERE file_id = ?  ORDER BY user_id)" );

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return receivers;
    }
    if(!stmt->bind(0, fileId))
    {
        MysqlPool::instance().releaseConnection(conn);
        return receivers;
    }
    try
    {
        auto result = stmt->query();
        while(result.fetch())
        {
            receivers.emplace_back( makeFileReceiver(result));////
        }
    }
    catch(const std::exception& e)
    {
        std::cerr<< "[FileReceiverModel] findByFileId failed: "<< e.what()<< std::endl;

        MysqlPool::instance().releaseConnection(conn);
        return {};
    }
    MysqlPool::instance().releaseConnection(conn);
    return receivers;
}
bool FileReceiverModel::removeByFileId(int64_t fileId)
{
    auto conn = MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }

    auto stmt = conn->prepare(
        R"(DELETE FROM file_receiver
           WHERE file_id = ?)"
    );

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    if(!stmt->bind(0, fileId))
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    bool ok = stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}