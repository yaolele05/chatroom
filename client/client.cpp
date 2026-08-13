#include "client.h"
#include "filetransf.h"
#include "../minimuduo/net/eventloop.h"
#include <iostream>
#include <algorithm>

Client::Client(EventLoop* loop):loop_(loop),tcpClient_(std::make_unique<TcpClient>(loop))
{

}
Client::~Client()
{

}

bool Client::connect(const std::string& ip, uint16_t port)
{
    if(!tcpClient_->connect(ip,port))
    {
        return false;
    }
    connection_=tcpClient_->connection();
    std::cout<<"connection="
             <<(connection_?"ok":"null")
             <<std::endl;

    if(!connection_)
    {
        return false;
    }
    connection_->setMessageCallback(std::bind(&Client::onMessage,this,std::placeholders::_1));
    fileTransfer_ =std::make_unique<FileTransfer>(connection_);
    return true;
}
void Client::login(const std::string& username,const std::string& password)
{
    {
         std::lock_guard<std::mutex> lock(loginMutex_);
         loginFinished_=false;
         login_=false;
    }
    if(!connection_)
    return;

    Message msg;
    msg.setType(Messagetype::Login);
    msg.setSequence(sequence_++);
    msg.setSenderId(0);
    msg.setTimestamp(time(nullptr));
    auto& payload=msg.payload();
    payload["username"]=username;
    payload["password"]=password;
    connection_->send(msg);
}
void Client::logout()
{
    if(!connection_)
    return;

    if(!login_)
    {
        return;
    }

    Message msg;
    msg.setType(Messagetype::Logout);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    connection_->send(msg);
}
void Client::addFriend(uint32_t friendId)
{
    if(!connection_)
    return;
     

    Message msg;
    msg.setType(Messagetype::AddFriend);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["friendId"]=friendId;
    connection_->send(msg);
}
void Client::deleteFriend(uint32_t friendId)
{
       if(!connection_)
    return;
    
    Message msg;
    msg.setType(Messagetype::DeleteFriend);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["friendId"]=friendId;
    connection_->send(msg);
}
void Client::privateChat(uint32_t receiverId,const std::string& text)
{
    if(!connection_)
    return;

    Message msg;
    msg.setType(Messagetype::PrivateChat);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setReceiverId(receiverId);
    msg.setTimestamp(time(nullptr));
    auto& payload=msg.payload();
    payload["content"]=text;
    connection_->send(msg);
}
void Client::createGroup(const std::string& groupName,const std::string&description)
{
    Message msg;
    msg.setType(Messagetype::CreateGroup);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["groupName"]=groupName;
    msg.payload()["description"]=description;
    connection_->send(msg);
}
void Client::joinGroup(uint32_t groupId)
{
    Message msg;
    msg.setType(Messagetype::JoinGroup);
    msg.setSequence(sequence_++);
    msg.setTimestamp(time(nullptr));
    msg.setSenderId(userId_);
    msg.payload()["groupId"]=groupId;
    connection_->send(msg);
}
void Client::leaveGroup(uint32_t groupId)
{
    Message msg;
    msg.setType(Messagetype::LeaveGroup);
    msg.setSequence(sequence_++);
    msg.setTimestamp(time(nullptr));
    msg.setSenderId(userId_);
    msg.payload()["groupId"]=groupId;
    connection_->send(msg);
}
void Client::groupChat(uint32_t groupId,const std::string& text)
{
    Message msg;
    msg.setType(Messagetype::GroupChat);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    auto& payload=msg.payload();
    payload["groupId"]=groupId;
    payload["content"]=text;
    connection_->send(msg);
}
void Client::sendPrivateFile(uint32_t userId,const std::string& filename)
{
    if(!login_)
    return;

    if(fileTransfer_)
    {
        fileTransfer_->sendPrivateFile(userId,filename);

    }
}
void Client::sendGroupFile(uint32_t groupId,const std::string& filename)
{
    if(!login_)
    {
        return;
    }
    if(fileTransfer_)
    {
         fileTransfer_->sendGroupFile(groupId,filename);
    }
}
void Client::sendImage(uint32_t userId,const std::string& filename)
{
    sendPrivateFile(userId,filename);
}

void Client::registerUser(const std::string& username,const std::string& password)
{
    if(!connection_)
    {
        return;
    }
    Message msg;
    msg.setType(Messagetype::Register);
    msg.setSequence(sequence_++);
    msg.setSenderId(0);
    msg.setTimestamp(time(nullptr));

    auto& payload = msg.payload();
    payload["username"] = username;
    payload["password"] = password;
    connection_->send(msg);
}

bool Client ::isLogin() const
{
    return login_;
}
void Client::friendList()
{
    if(!connection_ || !login_)
    {
        return;
    }
    Message msg;
    msg.setType(Messagetype::FriendList);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["request"]="friendlist";
    connection_->send(msg);
}
void Client::groupList()
{
    if(!connection_ || !login_)
    {
        return;
    }

    Message msg;
    msg.setType(Messagetype::GroupList);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload() = nlohmann::json::object();

    connection_->send(msg);
}
void Client::disconnect()
{
    if(tcpClient_)
    {
        tcpClient_->disconnect();
    }
    login_=false;
     userId_=0;
    username_.clear();
}
void Client::quit()
{
    loop_->quit();
}
void Client::privateHistory(uint32_t userid)
{
    Message msg;
    msg.setType(Messagetype::HistoryRequest);
    msg.payload()["type"]=1;
    msg.payload()["peerId"]=userid;

    connection_->send(msg);

}
void Client::groupHistory(uint32_t gid)
{
    Message msg;
    msg.setType(Messagetype::HistoryRequest);
    msg.payload()["type"]=2;
    msg.payload()["groupId"]=gid;
    connection_->send(msg);
}
void Client::onMessage(const Message& msg)
{
    std::cout << "client receive message type="<< static_cast<int>(msg.type())<< std::endl;

    //std::cout << "[Client] payload = "<< msg.payload().dump(4)<< std::endl;
   switch(msg.type())
   {
    case Messagetype::RegisterResponse: handleRegister(msg); break;
    case Messagetype::LoginResponse:handleLogin(msg);
    break;
    case Messagetype::LogoutResponse:handleLogout(msg);
    break;
    case Messagetype::PrivateChat:handlePrivateChat(msg);
    break;
    case Messagetype::PrivateChatResponse:handlePrivateChat(msg);
    break;

    case Messagetype::GroupChat:handleGroupChat(msg);
    break;
    case Messagetype::GroupList:
    case Messagetype::GroupListResponse:
    handleGroupList(msg);
    break;
    case Messagetype::AddFriendResponse:
    case Messagetype::DeleteFriendResponse:
    case Messagetype::FriendListResponse:
    handleFriend(msg);
    break;

    case Messagetype::CreateGroupResponse:
    case Messagetype::JoinGroupResponse:
    case Messagetype::LeaveGroupResponse:
    case Messagetype::GroupChatResponse:
    handleGroup(msg);
    break;


    case Messagetype::FileStart:
    case Messagetype::FileChunk:
    case Messagetype::FileFinish:
      handleFile(msg);
      break;

    case Messagetype::FILE_ACK:
    case Messagetype::FILE_RESUME_REQUEST:
    case Messagetype::FILE_RESUME_RESPONSE:
    handleFile(msg);
    break;

    case Messagetype::HeartBeatResponse:
    handleHeartbeat(msg);
    break;
    case Messagetype::MessageAck:
    {
    auto& payload=msg.payload();

    int code=payload.value("code",-1);
    if(code != 0)
    {
        std::cout << "message send failed: "<< payload.value("message", "") << std::endl;
        break;
    }
     std::cout << "message send success" << std::endl;
     if(payload.contains("fileId"))
    {
        if(fileTransfer_)
    {
        auto stage = msg.payload().value("stage","");
      if(stage=="start" ||stage=="chunk" ||stage=="finish")
      {
        fileTransfer_->handleAck(msg);
       }
      else
      {
        std::cout<< "[Client] normal message ack"<< std::endl;
      }
     }
    }
    break;
    }
    case Messagetype::Error:
    {
    const auto& payload = msg.payload();
    std::cout << "server error: "<< payload.value("message", "")<< std::endl;
    break;
    }
    case Messagetype::HistoryResponse:
    handleHistory(msg);
    break;
    case Messagetype::OfflineFileNotify:
    {
     handleOfflineFileNotify(msg);
    break;
    }

    default:
    std::cout<<"unknow message type"<<std::endl;
    break;

   }
}
void Client::handleRegister(const Message& msg)
{
    const auto& payload = msg.payload();
     if(payload.value("code",-1)==0)
    {
        std::cout<<"Register success."<<std::endl;
    }
    else
    {
        std::cout<<"Register failed:" <<payload.value("message","") <<std::endl;
    }
}
void Client::handleLogin(const Message& msg)
{
     const auto& payload=msg.payload();
     bool success=payload.value("success",false);

     {
         std::lock_guard<std::mutex> lock(loginMutex_);
         loginFinished_=true;
     if(!success)
     {
        login_=false;
        std::cout<<"Login failed:"<<payload.value("reason","")<<std::endl;
     }
     else
     {
     login_=true;
     userId_=payload.value("userid",0u);
     username_=payload.value("username","");
      std::cout<<"Login success"<<std::endl;
     std::cout<<"user:"<<username_<<"("<<userId_<<")"<<std::endl;
     }

    }

     loginCv_.notify_one();
    
}
void Client::handleLogout(const Message& msg)
{
    const auto& payload = msg.payload();
    bool success = payload.value("success", false);
    if(!success)
    {
        std::cout<<"Logout failed:"<<payload.value("reason","")<<std::endl;

        return;
    }

    login_ = false;
    userId_ = 0;
    username_.clear();
    std::cout<<"Logout success"<<std::endl;
}
void Client::handlePrivateChat(const Message& msg)
{
   const auto& payload=msg.payload();
   std::string text=payload.value("content","");
   std::cout<<"[Private]"<<msg.senderId()<<":"<<text<<std::endl;

}
void Client::handleGroupChat(const Message& msg)
{
    const auto& payload=msg.payload();

    uint32_t groupId=payload.value("groupId",0u);
    std::string content=payload.value("content","");
    std::cout<<"[Group"<<groupId<<"]"<<msg.senderId()<<":"<<content<<std::endl;

}
void Client::handleFriend(const Message& msg)
{
   const auto& payload=msg.payload();
   switch(msg.type())
   {
    case Messagetype::AddFriendResponse:
    {
    int code=payload.value("code",-1);
    if(code==0)
    {
        std::cout<<"Add friend success"<<std::endl;
    } 
    else
    {
        std::cout<<"Add friend failed"<<std::endl;
        std::cout<<payload.value("reason","");
    }
    break;
    }
    case Messagetype::DeleteFriendResponse:
    {
     int code=payload.value("code",-1);
      if(code==0)
      {
        std::cout<<"Delete friend success" <<std::endl;
      }
      else
      {
        std::cout<<"Delete friend failed"<<std::endl;
      }
        break;
    }
    case Messagetype::FriendListResponse:
    {
        if(!payload.contains("friends") || !payload["friends"].is_array())
    {
        std::cout << "好友列表为空" << std::endl;
        break;
    }
     friends_= payload["friends"];
    if(friends_.empty())
    {
        std::cout << "你还没有好友" << std::endl;
        break;
    }
    std::cout << "\n========== 好友列表 ==========\n";
    for(const auto& friendUser : friends_)
    {
           bool online = friendUser.value("online", false);
        std::cout<< "id: " << friendUser.value("id", 0)<< "\nusername: " << friendUser.value("username", "")<< "\nnickname: " << friendUser.value("nickname", "")<< "\nstatus: " << friendUser.value("status", 0)
        << "\nonline: " << (online ? "在线" : "离线")<< '\n'
         << "\n-----------------------------\n";
    }
    std::cout << "==============================\n";
    std::cout << "\n0. 返回\n";
    std::cout << "请选择好友ID：" << std::flush;
    break;
    }
    
    default:
    break;
   }

}
void Client::handleGroup(const Message& msg)
{
    const auto& payload = msg.payload();
    int code = payload.value("code", -1);
    std::string message = payload.value("message", "");
    switch(msg.type())
    {
    case Messagetype::CreateGroupResponse:
        if(code == 0)
        {
            std::cout << "create group success\n";
            std::cout << "groupId="<< payload.value("groupId", 0)<< std::endl;
        }
        else
        {
            std::cout << "create group failed: "<< message<< std::endl;
        }
        break;

    case Messagetype::JoinGroupResponse:
        if(code == 0)
        {
            std::cout << "join group success\n";
        }
        else
        {
            std::cout << "join group failed: "<< message<< std::endl;
        }
        break;

    case Messagetype::LeaveGroupResponse:
        if(code == 0)
        {
            std::cout << "leave group success\n";
        }
        else
        {
            std::cout << "leave group failed: "<< message<< std::endl;
        }
        break;

    case Messagetype::GroupChatResponse:
        if(code == 0)
        {
            std::cout << "group message send success\n";
        }
        else
        {
            std::cout << "group message send failed: "<< message<< std::endl;
        }
        break;

    default:
        break;
    }
}
void Client::handleHeartbeat(const Message& msg)
{
   std::cout<<"heartbeat response"<<std::endl;   
}
void Client::handleFile(const Message& msg)
{
    switch(msg.type())
    {
        case Messagetype::FileStart:
        fileTransfer_->handleFileStart(msg);

        break;
        case Messagetype::FileChunk:
          fileTransfer_->handleFileChunk(msg);
        std::cout<<"receive file chunk"<<std::endl;
        break;
        case Messagetype::FileFinish:

        fileTransfer_->handleFileFinish(msg);
        std::cout<<"receive file finish"<<std::endl;
        break;
        case Messagetype::FILE_ACK:
          fileTransfer_->handleAck(msg);
        break;
        case Messagetype::FILE_RESUME_REQUEST:
           fileTransfer_->handleResumeRequest(msg);
        break;
        case Messagetype::FILE_RESUME_RESPONSE:
          fileTransfer_->handleResumeResponse(msg);
         break; 

    default:
        break;
    }
}
void Client::handleGroupList(const Message& msg)
{
    const auto& payload = msg.payload();
    int code = payload.value("code", -1);
    if(code != 0)
    {
        std::cout << "获取群列表失败 " << payload.value("message", "") << std::endl;
        return;
    }
    if(!payload.contains("groups") ||!payload["groups"].is_array())
    {
        std::cout << "群列表为空" << std::endl;
        return;
    }
   groups_ = payload["groups"];
    if(groups_.empty())
    {
        std::cout << "你没加入任何群组" << std::endl;
        return;
    }
    std::cout << "\n========== 我的群组 ==========\n";
    for(const auto& group : groups_)
    {
        std::cout<< "groupId: "<< group.value("groupId", 0) << "\nname: "<< group.value("name", "")<< "\ndescription: "<< group.value("description", "")<< "\nownerId: "<< group.value("ownId", 0)
        << "\n-----------------------------\n";
    }
    std::cout << "==============================\n";
    std::cout << "\n0. 返回\n";
    std::cout<<"请选择群ID："<<std::flush;
}

bool Client::waitLoginResult()
{
    std::unique_lock<std::mutex> lock(loginMutex_);
    loginCv_.wait(lock,[this]()
        {
        return loginFinished_;
        }
    );
    bool ok=login_;
    return ok;
}
void Client::handleHistory(const Message& msg)
{
    auto& payload=msg.payload();
    if(!payload.contains("message"))
    {
        std::cout<<"history empty\n";
    }
    auto messages=payload["message"];
    std::reverse(messages.begin(),messages.end());
  
    if(!messages.is_array())
    {
        std::cout<<"messages not array\n";
        return;
    }
    std::reverse(messages.begin(),messages.end());
    std::cout<<"======== 历史消息 ========\n";
    for(auto&it:messages)
    {

        uint32_t senderId=it["senderId"];
        std::string content=it["content"];
        std::cout<<"["<<senderId<<"]"<<content<<std::endl;
    }

    std::cout<<"==========================\n";

}

void Client::handlerequestDownload(int64_t fileId)
{
    if(!connection_)
        return;

    if(!login_)
        return;

    Message msg;

    msg.setType(Messagetype::FileDownloadRequest);
    msg.setSequence(sequence_++);
    msg.payload()["fileId"] = fileId;
    std::cout<< "[Client] send DownloadRequest"<< " fileId=" << fileId<< std::endl;

    connection_->send(msg);
}
/*
void Client::handleOfflineFileNotify(const Message& msg)
{

auto& payload = msg.payload();
auto fileId =payload["fileId"].get<int64_t>();
auto senderId =msg.senderId();
auto fileName =payload["fileName"].get<std::string>();
auto fileSize =payload["fileSize"].get<uint64_t>();
 
 bool ok= fileTransfer_->createReceiveTask(fileId,fileName,fileSize);

 if(ok)
 {
     handlerequestDownload(fileId);
     std::cout<<"收到离线文件"<<"\nfileId="<<fileId<<"\n发送者="<<senderId<<"\n文件名="<<fileName<<"\n大小="<<fileSize<<std::endl;

 }
 else
 {
  std::cout<<"[client] create receivertask failed\n";
 }
}
 */
void Client::handleOfflineFileNotify(const Message& msg)
{

  fileTransfer_->handleOfflineFileNotify(msg);
}
const std::vector<FileTransfer::PendingReceiveFile>& Client::pendingReceiveFiles() const
{
    return fileTransfer_->pendingReceiveFiles();
}

void Client::acceptFile(int64_t fileId)
{
    if(!fileTransfer_)
        return;
    fileTransfer_->acceptFile(fileId);
}