#pragma once
#include <optional>
#include "entity/fileinfo.h"
class FileModel
{
    public:
    bool insert(const FileInfo& file);
    bool update(const FileInfo& file);
    std::optional<FileInfo> find (std::uint64_t  fileid);

    bool remove(std::uint64_t fileid);

};