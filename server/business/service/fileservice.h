#pragma once
#include "../../session/usersession.h"
#include "../../model/filemodel.h"
#include <unordered_map>
#include <memory>
#include <fstream>
#include<atomic>
class FileService
{
    public:
    struct SendTask
    {
       
      uint64_t fileId{0};
      FileInfo file;
      std::ifstream stream;
      uint64_t offset{0};
      bool waitingAck{false};
      bool finished{false};
      std::shared_ptr<Session> receiver;
    };
     struct ReceiveTask
    {
        FileInfo file;
        std::fstream stream;
        uint64_t offset{0};
    };
    static  FileService& instance();
    static void registerHandler();

    void fileStart(const Message& msg,Session* se);
    void fileChunk(const Message& msg,Session*se);
    void fileFinish(const Message& msg,Session* se);
    void fileAck(const Message& msg, Session* se);
    void sendFileToReceiver(const FileInfo& file,const std::shared_ptr<Session>& receiver);
    void sendNextChunk(SendTask& task);
    void downloadRequest(const Message& msg, Session* se);

    private:
  
    FileService()=default;

    std::unordered_map<uint64_t, std::unordered_map<int, std::unique_ptr<SendTask>>>sendTasks_;
      std::unordered_map<uint64_t, std::unique_ptr<ReceiveTask>> receiveTasks_;
};