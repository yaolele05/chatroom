#pragma once
#include "../../../common/protocol/protocol.h"
#include "../../session/usersession.h"
#include "../../model/filemodel.h"
class FileService
{
    public:
    static  FileService& instance();
    static void registerHandler();

    void fileStart(const Message& msg,Session* se);
    void fileChunk(const Message& msg,Session*se);
    void fileFinish(const Message& msg,Session* se);
    void sendFileToReceiver(const FileInfo& file, const std::shared_ptr<UserSession>& receiver);
    private:
    FileService()=default;
};