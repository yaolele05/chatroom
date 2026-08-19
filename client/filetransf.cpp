#include "filetransf.h"
#include "../common/protocol/Jsoncodec.h"
#include "../common/protocol/packetcodec.h"
#include <filesystem>
#include <iostream>
#include <atomic>
#include "../common/security/crypto/sha256.h"
#include "../common/security/crypto/base64.h"
#include <algorithm>//removePendingReceiveFile(int64_t fileId)
FileTransfer::FileTransfer(std::shared_ptr<ClientConnection> conn):connection_(std::move(conn))
{

}
bool FileTransfer::sendPrivateFile(uint32_t receiverId,const std::string& filename)
{
   
    
    std::ifstream fin(filename,std::ios::binary);
    if(!fin)
    {
      std::cout<<"open file failed\n";
       return false;
    }
    //计算文件大小
    fin.seekg(0,std::ios::end);
   uint64_t filesize = fin.tellg();
    fin.seekg(0,std::ios::beg);
   //计算文件sha
    std::string sha256 = Sha256::file(filename);
    if(sha256.empty())
    {
        std::cout << "calculate sha256 failed\n";
        return false;
    }
    std::cout << "file size = "<< filesize << std::endl;
    std::cout << "sha256 = "  << sha256 << std::endl;
   

    std::filesystem::path path(filename);
      Message start;
      start.setType(Messagetype::FileStart);
      start.setReceiverId(receiverId);
      auto& payload = start.payload();
      
      payload["receiverId"]=receiverId;
      payload["groupId"]=0;
      payload["fileName"] = path.filename().string();
      payload["fileSize"] = filesize;
      payload["sha256"]=sha256;
     
      PendingFile pending;
    pending.type=FileTarType::PRIVATE;
    pending.receiverId = receiverId;
    pending.filename = filename;
    pending.filesize = filesize;
    pending.sha256 = sha256;

    pendingFile_ = pending;
    hasPendingFile_ = true;
      connection_->send(start);
    return true;
}
bool FileTransfer::sendGroupFile(uint32_t groupId,const std::string& filename)
{
   
    
    std::ifstream fin(filename,std::ios::binary);
    if(!fin)
    {
      std::cout<<"open file failed\n";
       return false;
    }
    //计算文件大小
    fin.seekg(0,std::ios::end);
   uint64_t filesize = fin.tellg();
    fin.seekg(0,std::ios::beg);
   //计算文件sha
    std::string sha256 = Sha256::file(filename);
    if(sha256.empty())
    {
        std::cout << "calculate sha256 failed\n";
        return false;
    }
    std::cout << "file size = "<< filesize << std::endl;
    std::cout << "sha256 = "  << sha256 << std::endl;
   

    std::filesystem::path path(filename);
      Message start;
      start.setType(Messagetype::GroupFileStart);
      start.setReceiverId(0);
      auto& payload = start.payload();
      
      payload["receiverId"]=0;
      payload["groupId"]=groupId;
      payload["fileName"] = path.filename().string();
      payload["fileSize"] = filesize;
      payload["sha256"]=sha256;
     
      PendingFile pending;
    pending.type=FileTarType::GROUP;
    pending.receiverId = 0;
    pending.groupId=groupId;
    pending.filename = filename;
    pending.filesize = filesize;
    pending.sha256 = sha256;

    pendingFile_ = pending;
    hasPendingFile_ = true;
      connection_->send(start);
    return true;
}
void FileTransfer::sendChunks(uint64_t fileId,uint64_t offset)
{
   
   
    //std::cout<< "[FileTransfer] sendChunks"<< " fileId=" << fileId<< " offset=" << offset<< std::endl;
    auto it = sendTasks_.find(fileId);
    if(it==sendTasks_.end())
    {
        //std::cout<<"[FileTransfer] send task not found"<<std::endl;
        return;
    }

    SendTask &task = it->second;
    if(task.finished)
    {
        return;
    }
    if(task.waitingAck)
    {
        //std::cout<<"[FileTransfer] waiting ACK"<<"fileId="<<fileId<<"offset="<<task.offset<<std::endl;
        return;
    }
    if(offset > task.filesize)
    {
        //std::cout<< "[FileTransfer] invalid offset"<< " offset=" << offset<< " filesize=" << task.filesize<< std::endl;
        return;
    }
    if(offset==task.filesize)
    {
        Message finish;
        finish.setType(Messagetype::FileFinish);
        finish.setReceiverId(task.receiverId);
        finish.payload()["fileId"]=task.fileId;
        finish.payload()["fileName"]=std::filesystem::path(task.filename).filename().string();
        finish.payload()["fileSize"]=task.filesize;
        finish.payload()["sha256"]=task.sha256;

        connection_->send(finish);
        task.finished=true;
        std::cout<<"[FileTransfer] file upload finished"<<"fileId="<<task.fileId<<std::endl;
        return;
    }
    constexpr size_t CHUNK_SIZE = 64 * 1024;
    std::vector<char> buffer(CHUNK_SIZE);
    task.file.clear();
    task.file.seekg(static_cast<std::streamoff>(offset),std::ios::beg);////seekg?
   
    if(!task.file)
    {
        std::cout<<"[FileTransfer] seek file failed\n";
        return;
    }
       
        task.file.read(buffer.data(),CHUNK_SIZE);

        std::streamsize n=task.file.gcount();
        if(n<=0)
        {
            std::cout<<"[FileTransfer] read file failed"<<std::endl;
            return;
        }
         std::vector<unsigned char> bytes(buffer.begin(),buffer.begin() + n);
         std::string encoded=Base64::encode(bytes);

          uint64_t chunkOffset = offset;
        Message chunk;
        chunk.setType(Messagetype::FileChunk);
        chunk.setReceiverId(task.receiverId);

        chunk.payload()["fileId"]=task.fileId;
        chunk.payload()["offset"]=chunkOffset;
        chunk.payload()["size"]=static_cast<uint64_t>(n);
        chunk.payload()["data"]=encoded;

        connection_->send(chunk);
          task.offset = chunkOffset + static_cast<uint64_t>(n);
        
        task.waitingAck=true;
    
      //  std::cout<<"[FileTransfer] send chunk"<<"fileId="<<task.fileId<<"offset="<<offset<<"size="<<n<<"nextOffset="<<task.offset<<"/"<<task.filesize<<std::endl;
  
}

void FileTransfer::handleFileStart(const Message& msg)
{
     uint64_t fileId = msg.payload()["fileId"];
     bool accepted = false;
    for(const auto& file : pendingReceiveFiles_)
   {
    if(file.fileId == fileId)
    {
        accepted = file.accepted;
        break;
    }
   }
   if(!accepted)
   {
    std::cout<< "[FileTransfer] "<< "unexpected FileStart"<< " fileId=" << fileId<< std::endl;
    return;
   }
    ReceiveTask task;
    task.filename = msg.payload()["fileName"];
    task.filesize =msg.payload()["fileSize"];
    task.sha256 =msg.payload().value("sha256","");
     task.received = 0;
    std::cout<< "[FileTransfer] handleFileStart"<< " fileId=" << fileId<< " filename=" <<task.filename<< " filesize=" << task.filesize<< std::endl;
    
        std::filesystem::create_directories(saveDirectory_);
    std::filesystem::path path =std::filesystem::path(saveDirectory_) /task.filename;
    task.filepath = path.string();

    std::cout<< "[FileTransfer] create file: "<< task.filepath<< std::endl;
    task.file.open(path,std::ios::binary|std::ios::out|std::ios::trunc);//trunc??
    if(!task.file)
    {
        std::cout<<"[FileTransfer] create file failed:"<<task.filepath<<std::endl;
        return;
    }

   receiveTasks_.emplace(fileId,std::move(task));
    std::cout<< "[FileTransfer] start receive "<< "fileId=" << fileId<< " filename="<< msg.payload()["fileName"]<< " size="<< msg.payload()["fileSize"]<< std::endl;

   
    //// 告诉服务器：文件接收任务已经创建好了，从 offset=0 开始发送。
    Message ack;
    ack.setType(Messagetype::MessageAck);
    ack.setSequence(msg.sequence());
    ack.payload()["code"] = 0;
    ack.payload()["fileId"] = fileId;
    ack.payload()["offset"] = 0;
    ack.payload()["stage"]="start";

    connection_->send(ack);
   
}
void FileTransfer::handleFileChunk(const Message& msg)
{

    auto& payload = msg.payload();
    if(!payload.contains("fileId"))
    {
    return;
    }
     uint64_t fileId = payload.at("fileId").get<uint64_t>();  
     auto it=receiveTasks_.find(fileId);
       if(it==receiveTasks_.end())
       {
         std::cout<< "[FileTransfer] receive task not found" << " fileId="<< fileId<< std::endl;
       return;
       }

       auto& task=it->second;
         
       uint64_t offset =msg.payload().value("offset",task.received);
      if(offset != task.received)
     {
        std::cout<< "file offset wrong, expected="<< task.received<< " actual="<< offset<< std::endl;
        return;
     }

      uint64_t size=msg.payload()["size"].get<uint64_t>();
     if(offset + size > task.filesize) { std::cout << "[FileTransfer] file size overflow" << " offset=" << offset << " size=" << size << " filesize=" << task.filesize << std::endl; return; }
       //base64
       std::string encoded=msg.payload()["data"].get<std::string>();
     //  std::cout<< "[FileTransfer] handleFileChunk"<< " fileId=" << fileId<< " offset=" << offset<< " encodedSize=" << encoded.size()<< std::endl;
       
       std::vector<unsigned char>bytes=Base64::decode(encoded);
        if(bytes.empty() && !encoded.empty())
      {
        std::cout << "[FileTransfer] base64 decode failed\n";
        return;
      }
      if(bytes.size() != size)
    {
       std::cout<< "[FileTransfer] chunk size mismatch"<< " expected=" << size<< " actual=" << bytes.size()<< std::endl;
       return;
    }
       task.file.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());
        if(!task.file)
      {
        std::cout<< "write file failed\n";
        return;
      }
      
       task.received +=bytes.size();
       task.file.flush();
      std::cout<< "[FileTransfer] receive chunk"<< " fileId=" << fileId<< " received=" << task.received<< "/" << task.filesize<< std::endl;
      
       Message ack; 
       ack.setType(Messagetype::FILE_ACK);///接收客户端ack：：fileack
       ack.payload()["fileId"] = fileId; 
       ack.payload()["offset"] = task.received; 
       ack.payload()["stage"] = "chunk";
       connection_->send(ack); 
      std::cout << "[FileTransfer] send FILE_ACK" << " fileId=" << fileId << " offset=" << task.received << std::endl;

}
void FileTransfer::handleFileFinish(const Message& msg)
{
     uint64_t fileId = msg.payload()["fileId"];
      auto it=receiveTasks_.find(fileId);
       if(it==receiveTasks_.end())
       {
        std::cout<< "[FileTransfer] receive task not found"<< " fileId=" << fileId<< std::endl;
        return;
       }
       
       ReceiveTask&task=it->second;
   //    std::cout<< "[FileTransfer] handleFileFinish"<< " fileId=" << fileId<< " received=" << task.received<< "/" << task.filesize<< std::endl;
      if(task.received != task.filesize)
       {
         std::cout<< "[FileTransfer] file size not complete"<< std::endl;
        return;
       }
     task.file.flush();
      task.file.close();
      std::string actualSha256=Sha256::file(task.filepath);
      std::string expectedSha256=msg.payload().value("sha256","");
      if(expectedSha256.empty())
     {
       std::cout<< "[FileTransfer] missing sha256"<< std::endl;
       return;
     }
    if( actualSha256 != expectedSha256)
    {
       std::cout<< "[FileTransfer] sha256 verify failed"<< std::endl;
         return;
    }
     // std::cout<< "[FileTransfer] file receive success"<< std::endl;
      //std::cout<< "[FileTransfer] receive file: "<< task.filename<< std::endl;
      //std::cout<< "[FileTransfer] path: "<< task.filepath<< std::endl;
      //std::cout<< "[FileTransfer] expected sha256: "<< expectedSha256<< std::endl;
      //std::cout<< "[FileTransfer] actual sha256: "<< actualSha256<< std::endl;
      auto pendingIt =std::find_if(pendingReceiveFiles_.begin(),pendingReceiveFiles_.end(),[fileId](const PendingReceiveFile& file)
      {
        return file.fileId == fileId;
      }
    );

    if(pendingIt != pendingReceiveFiles_.end())
    {
    pendingReceiveFiles_.erase(pendingIt);
    }
     
   receiveTasks_.erase(it);
    
}
void FileTransfer::handleAck(const Message& msg)
{
   
   // std::cout<< "[FileTransfer] handleAck"<< " hasPendingFile="<< hasPendingFile_<< std::endl;

    const auto& payload=msg.payload();
    int code=payload.value("code",-1);
    if(code!=0)
    {
        std::cout<<"file start failed"<<payload.value("message","")<<std::endl;
        return;
    }
    if(!payload.contains("fileId"))
    {
        return;
    }
    uint64_t fileId = payload["fileId"].get<uint64_t>();
    std::string stage=payload.value("stage","");

    //std::cout<< "[FileTransfer] ACK" << " fileId=" << fileId<< " stage=" << stage<< std::endl;
    if(stage == "finish")
    {
    std::cout<< "[FileTransfer] FILE_FINISH ACK"<< " fileId=" << fileId<< std::endl;

       auto it = sendTasks_.find(fileId);
       if(it != sendTasks_.end())
      {
        it->second.finished = true;
        it->second.file.close();
        sendTasks_.erase(it);
    }

    return;
    }
     if(stage != "start" && stage != "chunk")
    {
       std::cout<< "[FileTransfer] unknown ACK stage"<< " stage=" << stage << std::endl;
       return;
    }
    uint64_t offset = payload.value("offset",0ULL);
   
    if(hasPendingFile_)
    {
         //std::cout<< "[FileTransfer] create SendTask"<< std::endl;
        std::ifstream fin(pendingFile_.filename,std::ios::binary);
        if(!fin)
        {
            std::cout<<"open failed\n";
            return;
        }
        if(offset > pendingFile_.filesize)
       {
        std::cout << "[FileTransfer] invalid ACK offset" << " offset=" << offset<< " filesize=" << pendingFile_.filesize<< std::endl;
          return;
       }
        SendTask task;
       task.fileId=fileId;
       task.receiverId=pendingFile_.receiverId;
       task.filename = pendingFile_.filename;
       task.filesize = pendingFile_.filesize;
       task.sha256=pendingFile_.sha256;
       task.file = std::move(fin);

       task.confirmedOffset = offset;
       task.offset = offset;
       task.waitingAck = false;
       task.finished=false;
     sendTasks_.emplace(fileId,std::move(task));
     hasPendingFile_=false;
    
    // std::cout<<"file create byserver"<<"fileId:"<<fileId<<std::endl;
     sendChunks(fileId,offset);
     return;
    }
    auto it = sendTasks_.find(fileId);
     if(it == sendTasks_.end())
      { 
        std::cout << "[FileTransfer]send task not found"<<"fileId=" << fileId << std::endl; 
        return;
      } 
      SendTask& task=it->second;
      if(offset > task.filesize)
    {
    std::cout<< "[FileTransfer] invalid ACK offset"<< " offset=" << offset<< " filesize=" << task.filesize<< std::endl;
       return;
    }

     if(offset < task.confirmedOffset)
    {
    std::cout<< "[FileTransfer] ACK offset rollback"<< " old=" << task.confirmedOffset<< " new=" << offset<< std::endl;

    return;
    }
      task.waitingAck=false;
      task.confirmedOffset = offset;
      task.offset=offset;

    std::cout<< "[FileTransfer] ACK"<< " fileId=" << fileId << " offset=" << offset<< "/"<< task.filesize<< std::endl;
    sendChunks(fileId,task.offset);
}
void FileTransfer::handleResumeRequest(const Message& msg)
{
    uint64_t fileId = msg.payload()["fileId"];
    auto it = sendTasks_.find(fileId);
    if(it == sendTasks_.end())
    {
        return;
    }
    Message response;
    response.setType(Messagetype::FILE_RESUME_RESPONSE);
    response.setReceiverId(it->second.receiverId);
    response.payload()["fileId"] = fileId;
    response.payload()["offset"] = it->second.confirmedOffset;
    connection_->send(response);
    // std::cout<< "[FileTransfer] resume request"<< " fileId=" << fileId<< " confirmedOffset="<< it->second.confirmedOffset<< std::endl;
}
void FileTransfer::handleResumeResponse(const Message& msg)
{
    uint64_t fileId = msg.payload()["fileId"];
    uint64_t offset = msg.payload()["offset"];
    auto it = sendTasks_.find(fileId);
    if(it == sendTasks_.end())
    {
         std::cout<< "[FileTransfer] send task not found"<< " fileId=" << fileId<< std::endl;
        return;
    }
    SendTask& task = it->second;
     task.waitingAck = false;
   task.confirmedOffset = offset;
   task.offset = offset;
  
   //std::cout<< "[FileTransfer] resume send"<< " fileId=" << fileId<< " offset=" << offset<< std::endl;

    sendChunks(fileId, offset);
}
void FileTransfer::requestDownload(int64_t fileId)
 {
    Message msg;
    msg.setType(Messagetype::FileDownloadRequest);
    msg.payload()["fileId"] = fileId;
    connection_->send(msg);
   // std::cout<< "[FileTransfer] request download "<< "fileId="<< fileId<< std::endl;
 }
 bool FileTransfer::createReceiveTask(uint64_t fileId,const std::string& filename,uint64_t filesize,const std::string& sha256)
{
  std::filesystem::create_directories(saveDirectory_);
    ReceiveTask task;
    task.fileId = fileId;
    task.filename = filename;
    task.filesize = filesize;
    task.received = 0;
    task.sha256 = sha256;
    task.filepath =(std::filesystem::path(saveDirectory_) / filename).string();
    task.file.open(task.filepath, std::ios::binary | std::ios::out | std::ios::trunc);
    if(!task.file)
    {
        std::cout<< "[FileTransfer] create file failed "<< task.filepath<< std::endl;

        return false;
    }
    receiveTasks_.emplace(fileId,std::move(task));
    std::cout<< "[FileTransfer] create receive task"<< " fileId=" << fileId<< " filename=" << filename<< " filesize=" << filesize<< std::endl;
    return true;
}
void FileTransfer::handleOfflineFileNotify(const Message& msg)//处理接收文件
{
    const auto& payload=msg.payload();
    if(!payload.contains("fileId")||!payload.contains("fileName")|| !payload.contains("fileSize"))
    {
      std::cout <<"[FileTransfer] invilid file notify"<<std::endl;
      return;
    }
    PendingReceiveFile file;
    file.fileId=payload["fileId"].get<int64_t>();
    file.senderId=msg.senderId();
    file.receiverId=msg.receiverId();
    file.filename=payload["fileName"].get<std::string>();
    file.groupId=payload.value("groupId",0);
    file.filesize=payload["fileSize"].get<uint64_t>();
    file.sha256=payload.value("sha256","");
    file.accepted=false;
    pendingReceiveFiles_.push_back(file);
    
  }
  const std::vector<FileTransfer::PendingReceiveFile>&FileTransfer::pendingReceiveFiles() const
{
    return pendingReceiveFiles_;
}

void FileTransfer::acceptFile(int64_t fileId)
{
    for( auto& file : pendingReceiveFiles_)
    {
      if(file.fileId != static_cast<uint64_t>(fileId))
        continue;
      if(file.fileId == fileId)
      {
      if(file.accepted)
      {
        std::cout<< "[FileTransfer] "<< "file already accepted" << std::endl;
        return;
      }
      if(!createReceiveTask(file.fileId,file.filename,file.filesize,file.sha256))
      {
        std::cout << "[FileTransfer] "<< "create receive task failed"<< " fileId=" << file.fileId << std::endl;
          return;
      }

       file.accepted = true;
      requestDownload(fileId);
     std::cout<< "[FileTransfer] "<< "accept file"<< " fileId=" << fileId<< std::endl;
            return;
        }
    }
    std::cout<< "[FileTransfer] "<< "pending file not found" << " fileId=" << fileId << std::endl;
}