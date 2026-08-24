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
   
    auto it = sendTasks_.find(fileId);
    if(it==sendTasks_.end())
    {
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
    size_t CHUNK_SIZE = 128 * 1024;
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
    

          uint64_t chunkOffset = offset;
        Message chunk;
        chunk.setType(Messagetype::FileChunk);
        chunk.setReceiverId(task.receiverId);

        chunk.payload()["fileId"]=task.fileId;
        chunk.payload()["offset"]=chunkOffset;
        chunk.payload()["size"]=static_cast<uint64_t>(n);
      

        connection_->send(chunk,buffer.data(),static_cast<size_t>(n));//yuanshi字节
          task.offset = chunkOffset + static_cast<uint64_t>(n);
        
        task.waitingAck=true;
  
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
             
     
         uint64_t received = 0;
    {
       std::lock_guard<std::mutex> lock(statemutex_);
       auto& task=it->second;
         
       uint64_t offset =msg.payload().value("offset",task.received);
      if(offset != task.received)
     {
        std::cout<< "file offset wrong, expected="<< task.received<< " actual="<< offset<< std::endl;
        return;
     }

      uint64_t size=msg.payload()["size"].get<uint64_t>();
     if(offset + size > task.filesize) { std::cout << "[FileTransfer] file size overflow" << " offset=" << offset << " size=" << size << " filesize=" << task.filesize << std::endl; return; }
      const std::string& bytes=msg.binary();
      if(bytes.size() != size)
     {
     std::cout<<"chunk size mismatch\n";
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
         received = task.received;
    }
       Message ack; 
       ack.setType(Messagetype::FILE_ACK);///接收客户端ack：：fileack
       ack.payload()["fileId"] = fileId; 
       ack.payload()["offset"] = received; 
       ack.payload()["stage"] = "chunk";
       connection_->send(ack); 
     

}
void FileTransfer::handleFileFinish(const Message& msg)
{
     uint64_t fileId = msg.payload()["fileId"];
       std::string filepath;
       std::string expectedSha256=msg.payload().value("sha256","");
        if(expectedSha256.empty())
     {
       std::cout<< "[FileTransfer] missing sha256"<< std::endl;
       return;
     }

   {
      std::lock_guard<std::mutex> lock(statemutex_);
      auto it=receiveTasks_.find(fileId);
       if(it==receiveTasks_.end())
       {
        std::cout<< "[FileTransfer] receive task not found"<< " fileId=" << fileId<< std::endl;
        return;
       }
       
       ReceiveTask&task=it->second;
      if(task.received != task.filesize)
       {
         std::cout<< "[FileTransfer] file size not complete"<< std::endl;
        return;
       }
     task.file.flush();
      task.file.close();
         filepath = task.filepath;

      }

      std::string actualSha256=Sha256::file(filepath);
    
    if( actualSha256 != expectedSha256)
    {
       std::cout<< "[FileTransfer] sha256 verify failed"<< std::endl;
         return;
    }
    
    {

     std::lock_guard<std::mutex> lock(statemutex_);
      auto pendingIt =std::find_if(pendingReceiveFiles_.begin(),pendingReceiveFiles_.end(),[fileId](const PendingReceiveFile& file)
      {
        return file.fileId == fileId;
      } );
  
    if(pendingIt != pendingReceiveFiles_.end())
    {
    pendingReceiveFiles_.erase(pendingIt);
    }
     auto it = receiveTasks_.find(fileId);

        if (it != receiveTasks_.end())
        {
            receiveTasks_.erase(it);
        }
  }
}
void FileTransfer::handleAck(const Message& msg)
{
   
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
    
}
void FileTransfer::handleResumeResponse(const Message& msg)
{
    uint64_t fileId = msg.payload()["fileId"];
    uint64_t offset = msg.payload()["offset"];
    auto it = sendTasks_.find(fileId);
    if(it == sendTasks_.end())
    {
           return;
    }
    SendTask& task = it->second;
     task.waitingAck = false;
   task.confirmedOffset = offset;
   task.offset = offset;

    sendChunks(fileId, offset);
}
void FileTransfer::requestDownload(int64_t fileId)
 {
    Message msg;
    msg.setType(Messagetype::FileDownloadRequest);
    msg.payload()["fileId"] = fileId;
    connection_->send(msg);
   
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
    std::filesystem::path filepath =std::filesystem::path(saveDirectory_) / filename;
   if(std::filesystem::exists(filepath))
   {
    std::string stem = filepath.stem().string();
    std::string exten = filepath.extension().string();
    int index = 1;
    while(std::filesystem::exists(filepath))
    {
        filepath = filepath.parent_path() /(stem + "(" + std::to_string(index) + ")" + exten);

        ++index;
    }
    }
    task.filepath = filepath.string();
    task.file.open(task.filepath, std::ios::binary | std::ios::out | std::ios::trunc);
    if(!task.file)
    {
   
        return false;
    }
    std::lock_guard<std::mutex> lock(statemutex_);
    receiveTasks_.emplace(fileId,std::move(task));

    return true;
}
void FileTransfer::handleOfflineFileNotify(const Message& msg)//处理接收文件
{
  
    const auto& payload=msg.payload();
    if(!payload.contains("fileId")||!payload.contains("fileName")|| !payload.contains("fileSize"))
    {
      
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

    
    std::lock_guard<std::mutex> lock(statemutex_);
    pendingReceiveFiles_.push_back(file);
   
  }
  std::vector<FileTransfer::PendingReceiveFile> FileTransfer::pendingReceiveFiles() 
{
  std::lock_guard<std::mutex> lock (statemutex_);
    return pendingReceiveFiles_;
}

bool FileTransfer::acceptFile(int64_t fileId)
{
  PendingReceiveFile target;
  {
    std::lock_guard<std::mutex> lock(statemutex_);
  
    for( auto& file : pendingReceiveFiles_)
    {
      if(file.fileId != static_cast<uint64_t>(fileId))
        continue;
      
      if(file.accepted)
      {
     
        return true;
      }
          target=file;
          break;
    }
  }
    if(target.fileId==0)
    {
      return false;
    }
      if(!createReceiveTask(target.fileId,target.filename,target.filesize,target.sha256))
      {
     
          return false;
      }
      
      {
        std::lock_guard<std::mutex> lock(statemutex_);
           for( auto& file : pendingReceiveFiles_)
        {
         if(file.fileId == static_cast<uint64_t>(fileId))
          {
            file.accepted = true;  
            break;
          }
        }
      }
     requestDownload(fileId);
           return true;
}
std::vector<FileTransfer::DownloadProgress> FileTransfer::downloadProgressList() 
{

   std::lock_guard<std::mutex> lock(statemutex_);
   std::vector<DownloadProgress>result;
   result.reserve(receiveTasks_.size());
   for(auto& [fileId,task]:receiveTasks_)
   {
    DownloadProgress progress;
    progress.filename=task.filename;
    progress.received=task.received;
    progress.total=task.filesize;
     result.push_back(progress);

   }
   return result;
}