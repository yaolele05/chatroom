#pragma once
#include <memory>
#include <fstream>
#include <atomic>
#include "clientconnection.h"
class FileTransfer
{
    public:
    enum class FileTarType
    {
       PRIVATE,
       GROUP
    };
    struct SendTask
   {
    uint64_t fileId{0};
    uint64_t serverFileId;//服务器数据库fileid
    uint32_t receiverId{0};
    uint32_t groupId{0};
    std::string filename;
    uint64_t filesize{0};
      std::string sha256;  
    uint64_t offset{0};
     uint64_t confirmedOffset{0};
    std::ifstream file;
    bool waitingAck{false};
    bool finished=false;
   };
    struct ReceiveTask
   {
    std::ofstream file;
     uint64_t fileId{0};
    std::string filename;
    uint64_t filesize{0};
    uint64_t received{0};
    std::string sha256;
   std::string filepath;
   };
   struct PendingFile
  {
    FileTarType type;
    uint32_t receiverId{0};
    uint32_t groupId{0};
    std::string filename;
    uint64_t filesize{0};
    std::string sha256;
  };
    explicit FileTransfer(std::shared_ptr<ClientConnection>conn);
    bool sendPrivateFile(uint32_t userId,const std::string& filename);
    bool sendGroupFile(uint32_t groupId,const std::string& filename);
    void sendChunks(uint64_t fileId,uint64_t offset);
    bool sendImage(uint32_t receiverId,const std::string& filename);

    void handleFileStart(const Message& msg);
    void handleFileChunk(const Message& msg);
    void handleFileFinish(const Message& msg);
    void handleAck(const Message& msg);
    void handleResumeRequest(const Message&msg);
    void handleResumeResponse(const Message&msg);
    void requestDownload(int64_t fileId);
    bool createReceiveTask(uint64_t fileId,const std::string& filename,uint64_t filesize);
    private:
     
    std::shared_ptr <ClientConnection> connection_; 
    std::string saveDirectory_{"download/"};
    std::unordered_map<uint64_t,SendTask> sendTasks_;
    std::unordered_map<uint64_t,ReceiveTask> receiveTasks_;
    PendingFile pendingFile_;
    bool hasPendingFile_{false};
};