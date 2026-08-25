#include "fileservice.h"
#include "../businessdispatcher/businessdispatcher.h"
#include "../../session/usersession.h"
#include"../../model/entity/fileinfo.h"
#include "../../model/filemodel.h"
#include "../../model/groupmodel.h"
#include <fstream>
#include"../../../common/protocol/message.h"
#include "../../session/sessionmanager.h"
#include "../../../common/security/crypto/sha256.h"
#include "../../model/offlinemodel.h"
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <fstream>
#include "../../model/filereceivermodel.h"
#include "../../model/entity/filereceiver.h"
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
   dispatcher.registerHandler(Messagetype::FileDownloadRequest,[](const Message& message,Session* session)
{
   FileService::instance().downloadRequest(message,session);   
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
    if(groupId != 0)
    {
    GroupModel groupModel;

    if(!groupModel.isGroupMember(groupId, senderId))
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "sender is not group member";

        se->send(reply);
        return;
    }
    }
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
       model.remove(file.id());

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
         if(receiverId == senderId)
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "cannot send file to yourself";
        se->send(reply);
        return;
    }
    FileReceiver receiverInfo;
    receiverInfo.setFileId(file.id());
    receiverInfo.setUserId(receiverId);
    receiverInfo.setStatus(FileReceiver::Waiting);
    receiverInfo.setCreateTime(
        std::chrono::system_clock::now()
    );

    FileReceiverModel receiverModel;

    if(!receiverModel.insert(receiverInfo))
    {
        model.remove(file.id());
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "create file receiver failed";

        se->send(reply);
        return;
    }
   }
    else{
      GroupModel groupModel;
     
      auto members= groupModel.findGroupMembers(groupId);
      std::cout<<"group members size="<<members.size()<<std::endl;

      bool receivercreatesuc=true;
      int receivercount=0;
      FileReceiverModel receiverModel;
      for(auto& member:members)
      {
     
        int memberId=member.userId();
        if(memberId==senderId)
        {
       

            continue;
        }
        ++receivercount;
         FileReceiver receiverInfo;

        receiverInfo.setFileId(file.id());
        receiverInfo.setUserId(member.userId());
        receiverInfo.setStatus(FileReceiver::Waiting);
        receiverInfo.setCreateTime(std::chrono::system_clock::now()
        );

        if(!receiverModel.insert(receiverInfo))
        {
            std::cout << "[FileService] insert group file_receiver failed"<< " fileId=" << file.id() << " userId=" << member.userId() << std::endl;
                receivercreatesuc=false;
            break;
        }
           
       
      
     }
     if(receivercount == 0)
       {
        
        receivercreatesuc = false;
       }
        if(!receivercreatesuc)
       {
       
        receiverModel.removeByFileId(file.id());

        // 删除 file
        model.remove(file.id());

        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "create file receivers failed";

        se->send(reply);

        return;
      }
   }
    Message reply;
   reply.setType(Messagetype::MessageAck);
   reply.setSequence(msg.sequence());
   reply.payload()["code"] = 0;
   reply.payload()["fileId"] = file.id();
   reply.payload()["offset"] = 0;
   reply.payload()["stage"]="start";

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
    

     std::int64_t fileId=payload.at("fileId").get<int64_t>();
     std::uint64_t offset=payload.at("offset").get<uint64_t>();
    const std::string& data=msg.binary();


     auto it = receiveTasks_.find(fileId);

    if(it == receiveTasks_.end())
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] = "file task not found";

        se->send(reply);
        return;
    }
    if(!it->second)
    {
  
    return;
    }
    ReceiveTask& task = *it->second;
   

    //////////////
    if(task.file.senderId() != senderId)
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());

        reply.payload()["code"] = -1;
        reply.payload()["message"] ="permission denied";
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
  
    if(data.empty())
    {
        Message reply;
        reply.setType(Messagetype::Error);
         reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="file offset chunk empty";
          
        se->send(reply);
        return;
    }

    if(task.offset + data.size() > task.file.fileSize())//溢出检查
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

    task.stream.write(data.data(),static_cast<std::streamsize>(data.size()));
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
    task.offset += data.size();
    
    

     Message reply;
     reply.setType(Messagetype::MessageAck);
     reply.setSequence(msg.sequence());
     reply.payload()["code"]=0;
     reply.payload()["fileId"] = fileId;
     reply.payload()["offset"]=task.offset;
     reply.payload()["stage"]="chunk";

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
    std::cout << "[DEBUG] 1 before findById" << std::endl;
     auto file= model.findById(fileId);

   std::cout << "[DEBUG] 2 after findById" << std::endl;
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
        reply.payload()["message"] ="file task not found";

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

    std::cout << "[DEBUG] 3 before completeFile" << std::endl;

     bool ok=model.completeFile(fileId);
     std::cout << "[DEBUG] 4 after completeFile" << std::endl;
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

     FileReceiverModel receiverModel;
std::cout << "[DEBUG] 5 before findByFileId" << std::endl;
   auto receivers = receiverModel.findByFileId(fileId);
   std::cout << "[DEBUG] 6 after findByFileId" << std::endl;
 
  for(auto& receiverInfo : receivers)
  {
    int receiverId = receiverInfo.userId();
    int status = receiverInfo.status();

 
    if(status == FileReceiver::Finished)
    {
      

        continue;
    }

    if(status == FileReceiver::Downloading)
    {
       
        continue;
    }  
    if(status != FileReceiver::Waiting)
    {
        
    }
    auto receiver =SessionManager::instance().getSession(receiverId);

    if(!receiver)
    {
      
     
        continue;
    }
     
    Message notify;
    notify.setType(Messagetype::OfflineFileNotify);
    notify.setSenderId(senderId);
    notify.setReceiverId(receiverId);
    notify.payload()["fileId"]=fileId;
    notify.payload()["fileName"]=file->fileName();
    notify.payload()["fileSize"]=file->fileSize();
    notify.payload()["sha256"]=file->fileSha256();
    notify.payload()["groupId"]=file->groupId();
    receiver->send(notify);
  
   }

    Message ack;
    ack.setType(Messagetype::MessageAck);
    ack.setSequence(msg.sequence());
    ack.payload()["code"] = 0;
     ack.payload()["message"] = "success";
     ack.payload()["fileId"]=fileId;
     ack.payload()["stage"]="finish";
    se->send(ack);
  return;
}
void FileService::sendFileToReceiver(const FileInfo& file,const std::shared_ptr<Session>& receiver)
{
    if(!receiver)
    {
     
        return;
    }

    auto userSession =std::dynamic_pointer_cast<UserSession>(receiver);

    if(!userSession)
    {

        return;
    }
    int receiverId = userSession->userid();
      auto& receiverTasks = sendTasks_[file.id()];
    // 防止同一个接收者重复创建发送任务
    auto existing = receiverTasks.find(receiverId);

    if(existing != receiverTasks.end())
    {
        std::cout<< "[FileService] send task already exists"<< " fileId=" << file.id()<< " receiverId=" << receiverId<< std::endl;
        return;
    }

    auto task = std::make_unique<SendTask>();

    task->fileId = file.id();
    task->file = file;
    task->offset = 0;
    task->waitingAck = false;
    task->finished = false;
    task->receiver = receiver;

    task->stream.open(file.filePath(),std::ios::binary);

    if(!task->stream)
    {
       
        return;
    }

    auto [it, inserted] =receiverTasks.emplace(receiverId,std::move(task));

    if(!inserted)
    {
      
        return;
    }

    SendTask& sendTask = *it->second;

    sendNextChunk(sendTask);
   
}

void FileService::fileAck(const Message& msg, Session* se)
{
    if(se == nullptr)
    {
        return;
    }
    if(!se->authenticated())
    {
        return;
    }
    auto userSession = dynamic_cast<UserSession*>(se);
    if(!userSession)
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
    if(!payload.contains("stage"))
    { 
    return;
    }
     std::string stage =payload.at("stage").get<std::string>();

      if(stage != "chunk")
    {

    return;
     }
    uint64_t fileId =payload.at("fileId").get<uint64_t>();
    uint64_t offset =payload.at("offset").get<uint64_t>();

    int receiverId = userSession->userid();
    auto fileIt = sendTasks_.find(fileId);
    if(fileIt == sendTasks_.end())
    {
      

        return;
    }

    auto& receiverTasks = fileIt->second;
    auto taskIt = receiverTasks.find(receiverId);

    if(taskIt == receiverTasks.end())
    {
      
        return;
    }

    SendTask& task = *taskIt->second;

    if(!task.receiver)
    {
       
        return;
    }

    bool permission = false;

    if(task.file.groupId() == 0)
    {
        permission =task.file.receiverId() == receiverId;
    }
    else
    {
        GroupModel model;
        permission =model.isGroupMember(task.file.groupId(), receiverId);
    }

    if(!permission)
    {
      
        return;
    }

    if(task.finished)
    {
        return;
    }

    if(!task.waitingAck)
    {
        //std::cout<< "[FileService] unexpected ACK"<< " fileId=" << fileId<< " receiverId=" << receiverId<< std::endl;
        return;
    }

    if(offset < task.offset ||offset > task.file.fileSize())
    {
        std::cout<< "[FileService] invalid ACK offset"<< " fileId=" << fileId<< " receiverId=" << receiverId<< " current=" << task.offset<< " ack=" << offset<< std::endl;
        return;
    }
    task.offset = offset;
    task.waitingAck = false;


  
    if(task.offset == task.file.fileSize())
    {
        int64_t groupId =task.file.groupId();
        uint64_t completedFileId =task.file.id();

        FileReceiverModel receiverModel;

        bool statusOk = receiverModel.updateStatus(completedFileId,receiverId,FileReceiver::Finished);

        if(!statusOk)
        {
          
            return;
        }       
        Message finish;
     
         finish.setType(Messagetype::FileFinish);
        finish.setSenderId(task.file.senderId());
        finish.setReceiverId( receiverId);

        finish.payload()["fileId"] =task.file.id();
        finish.payload()["fileName"] =task.file.fileName();
        finish.payload()["fileSize"] =task.file.fileSize();
        finish.payload()["sha256"] =task.file.fileSha256();
        task.receiver->send(finish);
        task.finished =true;

       
        task.stream.close();
        receiverTasks.erase(taskIt);
        std::cout<< "[FileService] erase receiver task"<< " fileId=" << fileId<< " receiverId=" << receiverId<< std::endl;
        if(receiverTasks.empty())
        {
            sendTasks_.erase(fileIt);
            std::cout<< "[FileService] erase file send task"<< " fileId=" << fileId<< std::endl;
        }
         //std::cout<< "[FileService] send FILE_FINISH"<< " fileId=" << fileId<< " receiverId=" << receiverId<< std::endl;
        return;
    }
    sendNextChunk(task);
}
void FileService::sendNextChunk(SendTask& task)
{
    if(!task.receiver)
    {
    return;
    }
    if(task.finished)
        return;
    auto userSession=std::dynamic_pointer_cast<UserSession>(task.receiver);

    if(!userSession)
    {
      
        return;
    }
    int receiverId=userSession->userid();
     // 文件已经全部发送
    if(task.offset >= task.file.fileSize())
    {
        Message finish;
        finish.setType(Messagetype::FileFinish);
        finish.setSenderId(task.file.senderId());
        finish.setReceiverId(receiverId);
        finish.payload()["fileId"] = task.file.id();
        finish.payload()["fileName"] = task.file.fileName();
        finish.payload()["fileSize"] = task.file.fileSize();
        finish.payload()["sha256"] = task.file.fileSha256();

        task.receiver->send(finish);
        task.finished = true;
        std::cout<< "[FileService] send FILE_FINISH"<< " fileId=" << task.file.id()<< std::endl;

        return;
    }
    size_t CHUNK_SIZE = 1024 * 1024;
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
    
   

    Message chunk;
    chunk.setType(Messagetype::FileChunk);
    chunk.setSenderId(task.file.senderId());
   chunk.setReceiverId(receiverId);
    auto& chunkPayload =chunk.payload();
    chunkPayload["fileId"] =task.file.id();
    chunkPayload["offset"] =task.offset;
    chunkPayload["size"] =static_cast<uint64_t>(n);
    
    task.receiver->send(chunk,buffer.data(),static_cast<size_t>(n));
    task.waitingAck = true;
   // std::cout<< "[FileService] send chunk"<< " offset=" << task.offset<< "/" << task.file.fileSize()<< std::endl;
}
void FileService::downloadRequest(const Message& msg,Session*se)
{
    if(!se)
    return;
    if(!se->authenticated())
    return;
     auto userSession =dynamic_cast<UserSession*>(se);
     if(!userSession)
     return;

      if(!msg.payload().contains("fileId"))
    return;
    auto fileId =msg.payload().at("fileId").get<int64_t>();
   

    FileModel model;
    auto file =model.findById(fileId);
    if(!file)
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"]=-1;
        reply.payload()["message"]="file not exist";
        se->send(reply);
        return;
    }
    if(!file->completed())
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"]=-1;
        reply.payload()["message"]="file not completed";
        se->send(reply);
        return;
    }
    bool pem=false;
    int receiverId=userSession->userid();

    if(file->groupId()==0)
    {
        pem=  file->receiverId()==receiverId;
    
    }
    else
   {
    GroupModel gmodel;

    pem=gmodel.isGroupMember(file->groupId(), receiverId);
 
    }
    if(!pem)
    {
        std::cout<<"[FileD=Service]download permission denied"<<"fileId="<<fileId<<"receiverId"<<receiverId<<std::endl;
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"]=-1;
        reply.payload()["message"]="download permission denied\n";
        se->send(reply);
        return;
    }
   

    FileReceiverModel receiverModel;
    auto receivers=receiverModel.findByFileId(fileId);
    
    FileReceiver* targetReceiver=nullptr;
    for(auto& receiver:receivers)
    {
        if(receiver.userId()==receiverId)
        {
            targetReceiver=&receiver;
            break;
        }
    }
    if(targetReceiver ==nullptr)
    {
        std::cout<<"[FileService] file_receiver not found"<<std::endl;
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="file receiver not found";

        se->send(reply);
        return;

    }

    int status=targetReceiver->status();
    if(status==FileReceiver::Finished)
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="file already download\n";

        se->send(reply);
        return;

    }
    if(status==FileReceiver::Downloading)
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="file already download\n";

        se->send(reply);
        return;
    }
    if(status!= FileReceiver::Waiting)
    {
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="invalid file receiver status\n";

        se->send(reply);
        return;
    }
    if(!receiverModel.updateStatus(fileId,receiverId,FileReceiver::Downloading))
    {
     
        Message reply;
        reply.setType(Messagetype::Error);
        reply.setSequence(msg.sequence());
        reply.payload()["code"] = -1;
        reply.payload()["message"] ="update file receiver status\n";
        return;

    }
     auto receiver =SessionManager::instance().getSession(receiverId);

    if(!receiver)
    {
    
        receiverModel.updateStatus(fileId,receiverId,FileReceiver::Waiting);
        return;
    }
    sendFileToReceiver(*file,receiver);
    
    Message ack;
    ack.setType(Messagetype::MessageAck);
    ack.setSequence(msg.sequence());
    ack.payload()["code"] = 0;
    ack.payload()["fileId"] =fileId;
    ack.payload()["offset"] =0;
    ack.payload()["message"] ="download started";
    se->send(ack);
}