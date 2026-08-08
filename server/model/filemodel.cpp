#include "filemodel.h"
#include "../database/connectionpool/mysqlpool.h"
#include "../database/mysql/mysqlclient.h"
#include "../database/mysql/mysqlstatement.h"
#include "../database/mysql/mysqlresult.h"

#include <chrono>
#include<optional>
FileInfo FileModel::makeFileInfo(
    MysqlResult& result)
{
    FileInfo file;
    file.setId( result.get<std::int64_t>(0));
    file.setSenderId(result.get<int32_t>(1));
    file.setReceiverId(result.get<int32_t>(2));
    file.setGroupId(result.get<int64_t>(3));
    file.setFileName(result.get<std::string>(4));
    file.setFilePath(result.get<std::string>(5));
    file.setFileSize(result.get<std::uint64_t>(6));
    file.setFileSha256(result.get<std::string>(7));
    file.setTransferredSize(result.get<std::uint64_t>(8));
    file.setCompleted(result.get<bool>(9));

    auto timestamp = result.get<std::int64_t>(10);
    file.setCreateTime(std::chrono::system_clock::time_point(std::chrono::seconds(timestamp)));
    return file;
}
bool FileModel::insert(FileInfo& file)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
       return false;
    }

    auto stmt=conn->prepare(R"(INSERT INTO file(sender_id,receiver_id,group_id,file_name,file_path,file_size,file_sha256,transferred_size,completed,create_time)VALUES(?,?,?,?,?,?,?,?,?,?))");

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    stmt->bind(0,file.senderId());
    stmt->bind(1,file.receiverId());
    stmt->bind(2,file.groupId());
    stmt->bind(3,file.fileName());
    stmt->bind(4,file.filePath());
    stmt->bind(5,file.fileSize());
    stmt->bind(6,file.fileSha256());
    stmt->bind(7,file.transferredSize());
    stmt->bind(8,file.completed());

    auto timestamp=std::chrono::duration_cast<std::chrono::seconds>(file.createTime().time_since_epoch()).count();

    stmt->bind(9,timestamp);

    bool ok=stmt->execute();
    if(ok)
    {
        file.setId(static_cast<int64_t>(conn->lastInsertId()));
    }

    MysqlPool::instance().releaseConnection(conn);

    return ok;
}
bool FileModel::updatetransfSize(std::int64_t fileId,std::uint64_t size)
{

    auto conn =MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
    auto stmt =
    conn->prepare(R"(UPDATE file SET transferred_size=? WHERE id=?)");

    if(!stmt)
    {
        MysqlPool::instance()
        .releaseConnection(conn);

        return false;
    }
    stmt->bind(0,size);
    stmt->bind(1,fileId);
    bool ok=stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
bool FileModel::completeFile( std::int64_t fileId)
{

    auto conn =MysqlPool::instance().getConnection();
    if(!conn)
        return false;
    auto stmt =conn->prepare(R"(UPDATE file SET completed=1 WHERE id=?)");
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0,fileId);
    bool ok = stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
std::optional<FileInfo> FileModel::findById(std::uint64_t fileid)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    return std::nullopt;

    auto stmt=conn->prepare(R"(SELECT id,sender_id,receiver_id,group_id,file_name,file_path,file_size,file_sha256,transferred_size,completed,create_time
        FROM file
        WHERE id=?)");
   if(!stmt)
   {
    MysqlPool::instance().releaseConnection(conn);
    return std::nullopt;
   }

   stmt->bind(0,fileid);

   auto result=stmt->query();
   std::optional<FileInfo> file;

   if(result.fetch())
   {
    file=makeFileInfo(result);
   }

   MysqlPool::instance().releaseConnection(conn);

   return file;

}
std::optional<FileInfo> FileModel::findBySha256(const std::string& sha256)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    return std::nullopt;

    auto stmt=conn->prepare(R"(SELECT id,sender_id,receiver_id,group_id,file_name,file_path,file_size,file_sha256,transferred_size,completed,create_time
        From file
        WHERE file_sha256=?)");
   if(!stmt)
   {
    MysqlPool::instance().releaseConnection(conn);
    return std::nullopt;
   }

   stmt->bind(0,sha256);

   auto result=stmt->query();
   std::optional<FileInfo> file;

   if(result.fetch())
   {
    file=makeFileInfo(result);
   }

   MysqlPool::instance().releaseConnection(conn);

   return file;

}
bool FileModel::updateFilePath(std::int64_t fileId,const std::string& path)
{
     auto conn = MysqlPool::instance().getConnection();
    if (!conn)
    return false;
    auto stmt = conn->prepare( R"(UPDATE file
           SET file_path = ?
           WHERE id = ?)");

    if (!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0, path);
    stmt->bind(1, fileId);
    bool ok = stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}