#pragma once

#include "entity/filereceiver.h"
#include <vector>
#include <optional>


class MysqlResult;


class FileReceiverModel
{
public:
    bool insert(FileReceiver& receiver);
    std::vector<FileReceiver> findWaitingFiles(int userid);
    bool updateStatus(int64_t fileId,int userid,int status);
    bool removeByFileId(int64_t fileId);
    std::vector<FileReceiver> findByFileId(int64_t fileId);


private:
    FileReceiver makeFileReceiver(MysqlResult& result);

};