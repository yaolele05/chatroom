#pragma once
#include <optional>
#include <vector>
#include "entity/fileinfo.h"

class MysqlResult;
class FileModel
{
    public:
    bool insert( FileInfo& file);
    bool updatetransfSize( std::int64_t fileId,std::uint64_t size);
    bool completeFile(std::int64_t fileId);

    bool updateFilePath(std::int64_t fileId,const std::string& path);
    std::optional<FileInfo> findById (std::uint64_t  fileid);
     std::optional<FileInfo> findBySha256 (const std::string& sha256);
    
     private:
     FileInfo makeFileInfo(MysqlResult& result);
};