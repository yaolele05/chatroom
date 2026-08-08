#include "fileservice.h"
#include "../businessdispatcher/businessdispatcher.h"
#include "../../session/usersession.h"
#include"../../model/entity/fileinfo.h"
#include "../../model/filemodel.h"
#include <fstream>
#include"../../../common/protocol/message.h"
#include "../../session/sessionmanager.h"
#include"../../../common/security/crypto/base64.h"
#include "../../../common/security/crypto/sha256.h"
#include "../../model/offlinemodel.h"
#include <iostream>
#include <filesystem>
FileService& FileService::instance()
{
    static FileService service;
    return service;
}
void FileService::registerHandler()
{

    auto& dispatcher=BusinessDispatcher::instance();
   dispatcher.registerHandler(Messagetype::FileStart,[]( const Message& message,Session* session)
{
    FileService::instance().fileStart(message,session);
});

   dispatcher.registerHandler(Messagetype::FileChunk,[](const Message& message,Session* session)
{
    FileService::instance().fileChunk(message,session);
});
   dispatcher.registerHandler(Messagetype::FileFinish,[](const Message& message,Session* session)
{
   FileService::instance().fileFinish(message,session);   
});
}
void FileService::fileStart(const Message& msg,Session*se)
{

    if(se==nullptr)
    return;

    if(!se->authenticated())
    return;

    auto userSession=dynamic_cast<UserSession*>(se);
    if(!userSession)
    return;

    int senderId=userSession->userid();

    auto& payload=msg.payload();
    if(!payload.contains("receiverId"))
    return;
    if(!payload.contains("groupId"))
    return;
    if(!payload.contains("fileName"))
    return;
    if(!payload.contains("fileSize"))
    return;
    if(!payload.contains("sha256"))
    return;
    
    int receiverId=payload.at("receiverId").get<int>();
    int groupId=payload.at("groupId").get<std::int64_t>();
    std::string fileName=payload.at("fileName").get<std::string>();
    std::uint64_t fileSize=payload.at("fileSize").get<uint64_t>();
    std::string sha256=payload.at("sha256").get<std::string>();

    FileInfo file;
    file.setSenderId(senderId);
    file.setReceiverId(receiverId);
    file.setGroupId(groupId);
    file.setFileName(fileName);
    file.setFileSize(fileSize);
    file.setFileSha256(sha256);
    file.setTransferredSize(0);
    file.setCompleted(false);
    file.setCreateTime(std::chrono::system_clock::now());


   FileModel model;
   if(!model.insert(file))
   {
    Message reply;
    reply.setType(Messagetype::Error);
    reply.setSequence(msg.sequence());
    reply.payload()["code"]=-1;
    reply.payload()["message"]="create file failed";

    se->send(reply);
    return;
   }

     std::filesystem::create_directories("files");
     std::string path ="files/"+std::to_string(file.id())+"_"+file.fileName();

     if(!model.updateFilePath(file.id(),path))
     {
           Message reply;
       reply.setType(Messagetype::Error);
       reply.setSequence(msg.sequence());
       reply.payload()["code"]=-1;
       reply.payload()["message"]="updatefilepath failed";
       reply.payload()["offset"] = 0;
        se->send(reply);
       return;

     }
     file.setFilePath(path);
     std::ofstream ofs(path,std::ios::binary);
     if(!ofs)
     {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
         reply.payload()["code"]=-1;
      reply.payload()["message"]="create file failed";
       se->send(reply);
       return;
     }

     ofs.close();
    
        Message reply;
     reply.setType(Messagetype::MessageAck);
     reply.setSequence(msg.sequence());
         reply.payload()["code"]=0;
         reply.payload()["fileId"]=file.id();
         reply.payload()["offset"]=0;

         se->send(reply);
}
void FileService::fileChunk(const Message& msg,Session* se)
{

    if(se==nullptr)
    return;

    if(!se->authenticated())
    return;

    auto userSession=dynamic_cast<UserSession*>(se);
    if(!userSession)
    return;
    int senderId=userSession->userid();

    auto& payload=msg.payload();
    if(!payload.contains("fileId"))
    return;
    if(!payload.contains("offset"))
    return;
    if(!payload.contains("data"))
    return;

     std::int64_t fileId=payload.at("fileId").get<int64_t>();
     std::uint64_t offset=payload.at("offset").get<uint64_t>();
     std::string data=payload.at("data").get<std::string>();

     FileModel model;
     auto file= model.findById(fileId);
     

     if(!file)
     {
        Message reply;
       reply.setType(Messagetype::Error);
       reply.setSequence(msg.sequence());
       reply.payload()["code"] = -1;
       reply.payload()["message"] = "file not exist";
       se->send(reply);
       return;
     }
     
     if(file->senderId() != senderId)
     {
      Message reply;
      reply.setType(Messagetype::Error);
      reply.setSequence(msg.sequence());
      reply.payload()["code"] = -1;
      reply.payload()["message"] = "permission denied";

      se->send(reply);
     return;
    }
     std::fstream fs(file->filePath(),std::ios::binary|std::ios::in|std::ios::out);

     if(!fs.is_open())
     {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"]=-1;
        reply.payload()["message"]="open file failed";
        se->send(reply);
        return;

     }
     
     if(offset != file->transferredSize())
     {

        Message reply;
       reply.setType(Messagetype::Error);
       reply.setSequence(msg.sequence());
       reply.payload()["code"] = -1;
       reply.payload()["message"] = "fileoffset wrong";
       se->send(reply);
       return;
     }
     fs.seekp(offset);
      auto bytes=Base64::decode(data);
      if(bytes.empty() && !data.empty())
    {
      Message reply;
      reply.setType(Messagetype::Error);
      reply.setSequence(msg.sequence());
      reply.payload()["code"] = -1;
      reply.payload()["message"] = "base64 decode failed";
      se->send(reply);
       return;
    }
      fs.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());
     model.updatetransfSize(fileId,offset+bytes.size());
     file->setTransferredSize(offset+bytes.size());


     Message reply;
     reply.setType(Messagetype::MessageAck);
     reply.setSequence(msg.sequence());
     reply.payload()["code"]=0;
     reply.payload()["offset"]=offset+bytes.size();

     se->send(reply);

 return;
}
void FileService::fileFinish(const Message& msg,Session* se)
{

    if(se==nullptr)
    return;

    if(!se->authenticated())
    return;

    auto userSession=dynamic_cast<UserSession*>(se);
    if(!userSession)
    return;

    int senderId=userSession->userid();
     auto& payload=msg.payload();
    if(!payload.contains("fileId"))
    return;


     FileModel model;
    std::int64_t fileId=payload.at("fileId").get<int64_t>();
     auto file= model.findById(fileId);
     if(!file)
     {
        Message reply;
       reply.setType(Messagetype::Error);
       reply.setSequence(msg.sequence());
       reply.payload()["code"] = -1;
       reply.payload()["message"] = "file not exist";
       se->send(reply);
       return;
     }

      if(file->senderId() != senderId)
     {
      Message reply;
      reply.setType(Messagetype::Error);
      reply.setSequence(msg.sequence());
      reply.payload()["code"] = -1;
      reply.payload()["message"] = "permission denied";

      se->send(reply);
     return;
    }
     
    std::string sha256=Sha256::file(file->filePath());

    if(sha256!=file->fileSha256())
    {
      Message reply;
      reply.setType(Messagetype::Error);
      reply.setSequence(msg.sequence());
      reply.payload()["code"]=-1;
      reply.payload()["message"]="sha256 verify failed";
      se->send(reply);
     return;
    }
     bool ok=model.completeFile(fileId);
     if(!ok)
     {
         Message reply;
       reply.setType(Messagetype::Error);
       reply.setSequence(msg.sequence());
       reply.payload()["code"] = -1;
       reply.payload()["message"] = "completefile fialed";
       se->send(reply);
       return;
     }

     Message okreply;
     okreply.setType(Messagetype::FileFinish);
     okreply.setSequence(msg.sequence());
     okreply.setSenderId(file->senderId());
     okreply.setReceiverId(file->receiverId());

    okreply.payload()["fileId"] = file->id();
    okreply.payload()["fileName"] = file->fileName();
    okreply.payload()["fileSize"] = file->fileSize();
    okreply.payload()["sha256"] = file->fileSha256();
    auto receiver=SessionManager::instance().getSession(file->receiverId());
    if(receiver)
    {
         sendFileToReceiver(*file, receiver);
    }
    
    else
   {
    OfflineMessage offline;
    offline.setUserId(file->receiverId());
    offline.setMessageId(file->id());
    offline.setType(OfflineType::File);
    offline.setCreateTime(std::chrono::system_clock::now());
    OfflineMessageModel offlinemodel;
    offlinemodel.insert(offline);
   }

    Message ack;
    ack.setType(Messagetype::MessageAck);
    ack.setSequence(msg.sequence());
    ack.payload()["code"] = 0;
     ack.payload()["message"] = "success";
    se->send(ack);
  return;
}
void FileService::sendFileToReceiver(const FileInfo& file,const std::shared_ptr<UserSession>& receiver)
{
    if(receiver == nullptr)
    {
        return;
    }
    Message start;

    start.setType(
        Messagetype::FileStart
    );

    start.setSenderId(
        file.senderId()
    );

    start.setReceiverId(
        file.receiverId()
    );


    auto& payload =
        start.payload();

    payload["fileId"] =
        file.id();

    payload["fileName"] =
        file.fileName();

    payload["fileSize"] =
        file.fileSize();

    payload["sha256"] =
        file.fileSha256();

    payload["offset"] =
        0;
    receiver->send(start);
    std::ifstream ifs(
        file.filePath(),
        std::ios::binary
    );
    if(!ifs)
    {
        std::cout<< "open server file failed: "<< file.filePath()<< std::endl;
        return;
    }
    constexpr size_t CHUNK_SIZE = 64 * 1024;

    std::vector<char> buffer(CHUNK_SIZE);
    uint64_t offset = 0;
    while(ifs)
    {
        ifs.read(buffer.data(),CHUNK_SIZE);
        std::streamsize n =ifs.gcount();
        if(n <= 0)
        {
            break;
        }
        std::vector<unsigned char> bytes(buffer.begin(),buffer.begin() + n);
        std::string encoded =Base64::encode(bytes);
        Message chunk;
        chunk.setType(Messagetype::FileChunk);
        chunk.setSenderId(file.senderId());
        chunk.setReceiverId(file.receiverId());
        auto& chunkPayload =chunk.payload();
        chunkPayload["fileId"] =file.id();
        chunkPayload["offset"] =offset;
        chunkPayload["size"] = static_cast<uint64_t>(n);
        chunkPayload["data"] =encoded;
        receiver->send(chunk);
        offset +=static_cast<uint64_t>(n);

        std::cout<< "server send file: "<< file.id()<< " offset="<< offset<< "/"<< file.fileSize()<< std::endl;
    }

    ifs.close();
    Message finish;
    finish.setType(Messagetype::FileFinish);
    finish.setSenderId(file.senderId());
    finish.setReceiverId(file.receiverId());
    finish.payload()["fileId"] =file.id();
    finish.payload()["fileName"] =file.fileName();
    finish.payload()["fileSize"] =file.fileSize();
    finish.payload()["sha256"] =file.fileSha256();
    receiver->send(finish);
    std::cout << "server send file finished, "<< "fileId="<< file.id()<< std::endl;
}