#include "fileservice.h"
#include "../businessdispatcher/businessdispatcher.h"
#include "../../session/usersession.h"
#include"../../model/entity/fileinfo.h"
#include "../../model/filemodel.h"
#include "../../model/groupmodel.h"
#include <fstream>
#include"../../../common/protocol/message.h"
#include "../../session/sessionmanager.h"
#include"../../../common/security/crypto/base64.h"
#include "../../../common/security/crypto/sha256.h"
#include "../../model/offlinemodel.h"
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <fstream>
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
dispatcher.registerHandler(Messagetype::GroupFileStart,[]( const Message& message,Session* session)
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
   dispatcher.registerHandler(Messagetype::FILE_ACK,[](const Message& message,Session* session)
{
   FileService::instance().fileAck(message,session);   
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

     auto task = std::make_unique<ReceiveTask>();
    task->file = file;
    task->stream.open(path,std::ios::binary |std::ios::in |std::ios::out |std::ios::trunc);
    if(!task->stream)
     {
       Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="write file failed";
        se->send(reply);
      return;
     }

    task->offset = 0;
    receiveTasks_[file.id()] =std::move(task);
    if(groupId==0)
    {
    auto receiver =SessionManager::instance().getSession(receiverId);
    if(!receiver)
    {
        std::cout<< "[FileService] receiver offline"<< " receiverId=" << receiverId<< std::endl;
        Message reply;

        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="receiver offline";
        se->send(reply);
        return;
    }
        Message start;
    start.setType(Messagetype::FileStart);
    start.setSenderId(senderId);
    start.setReceiverId(receiverId);
    start.payload()["fileId"] =file.id();
    start.payload()["fileName"] =file.fileName();
    start.payload()["fileSize"] =file.fileSize();
    start.payload()["sha256"] =file.fileSha256();
    start.payload()["offset"] =0;

    receiver->send(start);

    std::cout<< "[FileService] send FILE_START"<< " fileId=" << file.id()<< " senderId=" << senderId<< " receiverId=" << receiverId<< " size=" << file.fileSize()<< std::endl;

    }
    else{
      GroupModel groupModel;
     
      auto members= groupModel.findGroupMembers(groupId);
      std::cout<<"group members size="<<members.size()<<std::endl;
      for(auto& member:members)
      {
        std::cout<<"member userid="<<member.userId()<<std::endl;
        if(member.userId()==senderId)
        {
             std::cout<<"skip sender"<<std::endl;

            continue;
        }
        auto receiver=SessionManager::instance().getSession(member.userId());

        if(!receiver)
        {

            continue;
        }
        Message start;
        start.setType(Messagetype::GroupFileStart);
        start.setSenderId(senderId);
        start.setReceiverId(member.userId());

        start.payload()["fileId"]=file.id();
        start.payload()["groupId"]=groupId;
        start.payload()["fileName"]=file.fileName();
        start.payload()["fileSize"]=file.fileSize();
        start.payload()["sha256"]=file.fileSha256();
        start.payload()["offset"]=0;
        receiver->send(start);


        std::cout<< "[FileService] send GROUP FILE_START"<< " fileId="<< file.id()<< " userid="<< member.userId()<< std::endl;
      }

    }
        
        
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

     std::cout<< "[FileService] ENTER fileChunk"<< " this=" << this<< " fileId=" << msg.payload().value("fileId", 0ULL)<< " offset=" << msg.payload().value("offset", 0ULL)<< std::endl;
    
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

     auto it = receiveTasks_.find(fileId);

    if(it == receiveTasks_.end())
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] =
            "file task not found";

        se->send(reply);
        return;
    }
    ReceiveTask& task = *it->second;
    if(task.file.senderId() != senderId)
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());

        reply.payload()["code"] = -1;
        reply.payload()["message"] =
            "permission denied";
        se->send(reply);
        return;
    }
    // offset 必须连续
    if(offset != task.offset)
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="file offset wrong";
        reply.payload()["offset"] =task.offset;
        se->send(reply);
        return;
    }
    auto bytes = Base64::decode(data);
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
    if(task.offset + bytes.size() > task.file.fileSize())
    {
    Message reply;
    reply.setType(Messagetype::Error);
    reply.setSequence(msg.sequence());

    reply.payload()["code"] = -1;
    reply.payload()["message"] = "file size overflow";
    reply.payload()["offset"] = task.offset;
    se->send(reply);
    return;
   }

    task.stream.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
    if(!task.stream)
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="write file failed";
        se->send(reply);
        return;
    }
    task.offset += bytes.size();
    
    std::cout<< "[FileService] write chunk"<< " fileId=" << fileId<< " offset=" << offset<< " bytes=" << bytes.size()<< " nextOffset=" << task.offset<< std::endl;

     Message reply;
     reply.setType(Messagetype::MessageAck);
     reply.setSequence(msg.sequence());
     reply.payload()["code"]=0;
     reply.payload()["fileId"] = fileId;
     reply.payload()["offset"]=task.offset;

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
     auto it = receiveTasks_.find(fileId);
     if(it == receiveTasks_.end())
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] =
            "file task not found";

        se->send(reply);
        return;
    }

    ReceiveTask& task = *it->second;

   if(task.offset != file->fileSize())
   {
    Message reply;
    reply.setType(Messagetype::Error);
    reply.setSequence(msg.sequence());

    reply.payload()["code"] = -1;
    reply.payload()["message"] = "file not complete";
    reply.payload()["offset"] = task.offset;

    se->send(reply);
    return;
   }
    task.stream.flush();
    task.stream.close();
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

    receiveTasks_.erase(it);
    if(file->groupId()==0)
    {
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
    }
    else
    {
         GroupModel groupModel;
      auto members= groupModel.findGroupMembers(file->groupId());
      for(auto& member:members)
      {
        if(member.userId()==senderId)
        {
            continue;
        }
        auto receiver=SessionManager::instance().getSession(member.userId());
        if(!receiver)
        {
             OfflineMessage offline;
          offline.setUserId(member.userId());
          offline.setMessageId(file->id());
          offline.setType(OfflineType::File);
          offline.setCreateTime(std::chrono::system_clock::now());
          OfflineMessageModel offlinemodel;
          offlinemodel.insert(offline);
            continue;
        }
        else
        {
              sendFileToReceiver(*file, receiver);
        }
      }
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
    auto task = std::make_unique<SendTask>();
    task->fileId = file.id();
    task->file = file;
    task->offset = 0;
    task->waitingAck = false;
    task->finished = false;
    task->stream.open(file.filePath(),std::ios::binary);

    if(!task->stream)
    {
        std::cout<< "[FileService] open file failed: "<< file.filePath()<< std::endl;
        return;
    }
    auto [it, inserted] =sendTasks_.emplace(file.id(),std::move(task));
    if(!inserted)
    {
        std::cout<<"[FileService] send task already exists, fileId=" << file.id()<< std::endl;
        return;
    }

    SendTask& sendTask = *it->second;

    sendTask.waitingAck = false;
    sendNextChunk(sendTask, receiver.get());
   std::cout<< "[FileService] send file start"<< " fileId=" << file.id() << " size=" << file.fileSize()<< std::endl;
}
void FileService::fileAck(const Message& msg,Session* se)
{
    if(se == nullptr)
    {
        return;
    }
    if(!se->authenticated())
    {
        return;
    }
    const auto& payload = msg.payload();

    if(!payload.contains("fileId"))
    {
        return;
    }

    if(!payload.contains("offset"))
    {
        return;
    }

    uint64_t fileId =payload.at("fileId").get<uint64_t>();
    uint64_t offset =payload.at("offset").get<uint64_t>();

    auto it = sendTasks_.find(fileId);
    if(it == sendTasks_.end())
    {
        std::cout<< "[FileService] send task not found"<< " fileId=" << fileId<< std::endl;
        return;
    }

    SendTask& task = *it->second;
    auto userSession = dynamic_cast<UserSession*>(se);
    if(!userSession)
    {
    return;
    }

    bool permisson=false;
    if(task.file.groupId()==0)
    {
        permisson=task.file.receiverId()==userSession->userid();
    }
    else
    {
        GroupModel model;
        permisson=model.isGroupMember(task.file.groupId(),userSession->userid());
    }
    if(!permisson)
    {
         std::cout<< "[FileService] ACK permission denied"<< std::endl;

    return;
    }
    if(task.finished)
    {
        return;
    }
    
     if(offset < task.offset ||offset > task.file.fileSize())
    {
        std::cout<< "[FileService] invalid ACK offset"<< " fileId=" << fileId<< " current=" << task.offset<< " ack=" << offset<< std::endl;
        return;
    }
      task.offset = offset;
      task.waitingAck = false;

   std::cout<< "[FileService] ACK"<< " fileId=" << fileId<< " offset=" << offset<< std::endl;

    if(task.offset == task.file.fileSize())
    {
        Message finish;

        finish.setType(Messagetype::FileFinish);
        finish.setSenderId(task.file.senderId());
        finish.setReceiverId(task.file.receiverId());
        finish.payload()["fileId"] =task.file.id();
        finish.payload()["fileName"] =task.file.fileName();
        finish.payload()["fileSize"] =task.file.fileSize();
        finish.payload()["sha256"] =task.file.fileSha256();

        se->send(finish);

        task.finished = true;

        std::cout<< "[FileService] send FILE_FINISH" << " fileId=" << fileId<< std::endl;

        return;
    }

    sendNextChunk(task, se);
}
void FileService::sendNextChunk(SendTask& task, Session* se)
{
     if(se == nullptr)
        return;

    if(task.finished)
        return;

     // 文件已经全部发送
    if(task.offset >= task.file.fileSize())
    {
        Message finish;
        finish.setType(Messagetype::FileFinish);
        finish.setSenderId(task.file.senderId());
        finish.setReceiverId(task.file.receiverId());

        finish.payload()["fileId"] = task.file.id();
        finish.payload()["fileName"] = task.file.fileName();
        finish.payload()["fileSize"] = task.file.fileSize();
        finish.payload()["sha256"] = task.file.fileSha256();

        se->send(finish);

        task.finished = true;

        std::cout<< "[FileService] send FILE_FINISH"<< " fileId=" << task.file.id()<< std::endl;

        return;
    }
    constexpr size_t CHUNK_SIZE = 64 * 1024;
    std::vector<char> buffer(CHUNK_SIZE);

    task.stream.clear();
    task.stream.seekg(static_cast<std::streamoff>(task.offset),std::ios::beg);

    if(!task.stream)
    {
        std::cout<< "[FileService] seek failed"<< " fileId=" ;
        return;
    }
    task.stream.read(buffer.data(),CHUNK_SIZE);

    std::streamsize n =task.stream.gcount();

    if(n <= 0)
    {
        std::cout<< "[FileService] read file failed";
        return;
    }
    std::vector<unsigned char> bytes(buffer.begin(),buffer.begin() + n);
    std::string encoded =Base64::encode(bytes);

    Message chunk;
    chunk.setType(Messagetype::FileChunk);
    chunk.setSenderId(task.file.senderId());

   auto user =dynamic_cast<UserSession*>(se);
   chunk.setReceiverId(user->userid());
    auto& chunkPayload =chunk.payload();
    chunkPayload["fileId"] =task.file.id();
    chunkPayload["offset"] =task.offset;

    chunkPayload["size"] =static_cast<uint64_t>(n);
    chunkPayload["data"] =encoded;
    se->send(chunk);
    task.waitingAck = true;
    std::cout<< "[FileService] send chunk"<< " offset=" << task.offset<< "/" << task.file.fileSize()<< std::endl;
}