#pragma once
#include <optional>
#include <vector>
#include "entity/fileinfo.h"

class MysqlResult;
class FileModel
{
    public:
    bool insert( FileInfo& file);
  
    bool completeFile(std::int64_t fileId);

    bool updateFilePath(std::int64_t fileId,const std::string& path);
    std::optional<FileInfo> findById (std::uint64_t  fileid);
   
    bool remove(std::int64_t fileId);
    
     private:
     FileInfo makeFileInfo(MysqlResult& result);
};