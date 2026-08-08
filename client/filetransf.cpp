#include "filetransf.h"
#include "../common/protocol/Jsoncodec.h"
#include "../common/protocol/packetcodec.h"
#include <filesystem>
#include <iostream>
#include <atomic>
#include "../common/security/crypto/sha256.h"
#include "../common/security/crypto/base64.h"
FileTransfer::FileTransfer(std::shared_ptr<ClientConnection> conn):connection_(std::move(conn))
{

}
bool FileTransfer::sendFile(uint32_t receiverId,const std::string& filename)
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

    pending.receiverId = receiverId;
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
    std::cout
    << "[FileTransfer] sendChunks"
    << " fileId=" << fileId
    << " offset=" << offset
    << std::endl;
    auto it = sendTasks_.find(fileId);
    if(it==sendTasks_.end())
    {
        return;
    }

    SendTask &task = it->second;
    task.file.clear();
    task.file.seekg(static_cast<std::streamoff>(offset),std::ios::beg);////seekg?
   
    if(!task.file)
    {
        std::cout<<"seek file failed\n";
        return;
    }
     task.offset=offset;
    constexpr size_t CHUNK_SIZE = 64*1024;
     std::vector<char> buffer(CHUNK_SIZE);

    while(task.file)
    {
        task.file.read(buffer.data(),CHUNK_SIZE);
        std::streamsize n=task.file.gcount();
        if(n<=0)
        break;
         std::vector<unsigned char> bytes(buffer.begin(),buffer.begin() + n);
         std::string encoded=Base64::encode(bytes);

        Message chunk;
        chunk.setType(Messagetype::FileChunk);
        chunk.setReceiverId(task.receiverId);

        chunk.payload()["fileId"]=task.fileId;
        chunk.payload()["offset"]=task.offset;
        chunk.payload()["size"]=static_cast<uint64_t>(n);
        chunk.payload()["data"]=encoded;

        connection_->send(chunk);
        task.offset+=static_cast<uint64_t>(n);
        std::cout<<"send chunk:fileId"<<task.fileId<<"offset:"<<task.offset<<"filesize:"<<task.filesize<<std::endl;
    }
    if(task.offset>=task.filesize)
    {
        Message finish;
        finish.setType(Messagetype::FileFinish);
        finish.setReceiverId(task.receiverId);
        finish.payload()["fileId"]=task.fileId;
       finish.payload()["fileName"]=task.filename;
       finish.payload()["fileSize"]=task.filesize;
       finish.payload()["sha256"]=task.sha256;
       connection_->send(finish);

         task.finished=true;
         std::cout<<"fileupload finished,fileId:"<<task.fileId<<std::endl;

    }
}
bool FileTransfer::sendImage(uint32_t receiverId, const std::string& filename)
{
    return sendFile(receiverId,filename);
}
void FileTransfer::handleFileStart(const Message& msg)
{
     uint64_t fileId = msg.payload()["fileId"];

    ReceiveTask task;
    task.filename = msg.payload()["fileName"];
    task.filesize =msg.payload()["fileSize"];
    task.sha256 =msg.payload().value("sha256","");
     std::filesystem::path path =std::filesystem::path(saveDirectory_) /task.filename;
    std::filesystem::create_directories(saveDirectory_);
    task.filepath = path.string();
    task.file.open(path,std::ios::binary|std::ios::trunc);//trunc??
    if(!task.file)
    {
        std::cout<<"create file failed\n";
        return;
    }
    task.received=0;
    std::cout<< "start receive "<< task.filename<< " fileId="<< fileId<< std::endl;
    receiveTasks_.emplace(fileId,std::move(task));////
   

}
void FileTransfer::handleFileChunk(const Message& msg)
{
        uint64_t fileId=msg.payload()["fileId"];
     auto it=receiveTasks_.find(fileId);
       if(it==receiveTasks_.end())
       return;

       auto& task=it->second;
         
       uint64_t offset =msg.payload().value("offset",task.received);
      if(offset != task.received)
     {
        std::cout
            << "file offset wrong, expected="
            << task.received
            << " actual="
            << offset
            << std::endl;

        return;
     }
       //base64
       std::string encoded=msg.payload()["data"].get<std::string>();
       std::vector<unsigned char>bytes=Base64::decode(encoded);
        if(bytes.empty() && !encoded.empty())
      {
        std::cout
            << "base64 decode failed\n";

        return;
      }
       task.file.write(reinterpret_cast<const char*>(bytes.data()),bytes.size());
        if(!task.file)
      {
        std::cout<< "write file failed\n";
        return;
      }
       task.received +=bytes.size();

    std::cout<<"progress "<<task.received<<"/"<<task.filesize<<std::endl;

}
void FileTransfer::handleFileFinish(const Message& msg)
{
     uint64_t fileId = msg.payload()["fileId"];
      auto it=receiveTasks_.find(fileId);
       if(it==receiveTasks_.end())
       return;
       ReceiveTask&task=it->second;
      task.file.close();
      std::string actualSha256=Sha256::file(task.filepath);
      std::string expectedSha256=msg.payload().value("sha256","");

   std::cout<<"receive file:"<<task.filename<<std::endl;
   std::cout<<"expected sha256:"<<expectedSha256<<std::endl;
   std::cout<<"actual sha256:"<<actualSha256<<std::endl;
    if(!expectedSha256.empty() &&actualSha256 != expectedSha256)
    {
        std::cout<< "file sha256 verify failed"<< std::endl;
    }
    else
    {
        std::cout<< "file receive success"<< std::endl;
    }
   receiveTasks_.erase(it);
    
}
void FileTransfer::handleAck(const Message& msg)
{
    std::cout<< "[FileTransfer] handleAck"<< " hasPendingFile="<< hasPendingFile_<< std::endl;
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
    uint64_t offset = payload.value("offset",0ULL);
    if(hasPendingFile_)
    {
         std::cout<< "[FileTransfer] create SendTask"<< std::endl;
        std::ifstream fin(pendingFile_.filename,std::ios::binary);
        if(!fin)
        {
            std::cout<<"open failed\n";
            return;
        }
         
     SendTask task;
     task.fileId=fileId;
     task.receiverId=pendingFile_.receiverId;
     task.filename = pendingFile_.filename;
     task.filesize = pendingFile_.filesize;
     task.offset = offset;
     task.finished = false;
     task.sha256=pendingFile_.sha256;
     task.file = std::move(fin);

     sendTasks_.emplace(fileId,std::move(task));
     hasPendingFile_=false;
     std::cout<<"file create byserver"<<"fileId:"<<fileId<<std::endl;
     sendChunks(fileId,offset);

     return;
    }
    auto it = sendTasks_.find(fileId);
     if(it == sendTasks_.end())
      { 
        std::cout << "send task not found, fileId=" << fileId << std::endl; 
        return;
     } 
    it->second.offset = offset;
    std::cout<<"ACK file"<<fileId<<"offset="<<offset<<"filesize="<<it->second.filesize<<std::endl;
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
    response.payload()["offset"] = it->second.offset;
    connection_->send(response);
    std::cout<< "resume request file="<< fileId<< " offset="<< it->second.offset<< std::endl;
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
    std::cout<< "resume send file="<< fileId<< " offset="<< offset<< std::endl;

    sendChunks(fileId, offset);
}