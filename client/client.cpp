#include "client.h"
#include "filetransf.h"
#include "../minimuduo/net/eventloop.h"
#include <iostream>
#include <algorithm>
#include "../common/protocol/Jsoncodec.h"
#include <termios.h>
#include <unistd.h>
#include <iostream>
#include <string>

#include <unistd.h>

Client::Client(EventLoop* loop):loop_(loop),tcpClient_(std::make_unique<TcpClient>(loop))
{
   loop_->setTimerCallback([this]()
    {
        if(login_)
        {
            sendHeartbeat();
        }
    });
}
Client::~Client()
{

}
void Client::sendHeartbeat()
{
    if(!login_)
        return;
    if(!connection_ || !connection_->connected())
        return;
    Message msg;
    msg.setType(Messagetype::HeartBeat);
    connection_->send(msg);
}


void Client::handleHeartbeat(const Message& msg)
{
   //std::cout<<"heartbeat response"<<std::endl;   
}
void Client::printNotification(const std::string& message)
{
    std::lock_guard<std::mutex> lock(chatOutputMutex_);

    std::cout << "\n"<< "[通知] "<< message<< "\n";
}
constexpr int CHAT_WIDTH = 70;
void Client::printChatMessage( uint32_t senderId,uint32_t currentUserId,const std::string& senderName,const std::string& content)
{
    std::string text;
    if(senderId == currentUserId)
    {
     text = "我：" + content;
     int padding =CHAT_WIDTH - static_cast<int>(text.size());

     if(padding > 0)
     {
         std::cout << std::string(padding, ' ');
     }
        std::cout << text << '\n';
    }
    else
    {
    text = senderName + "：" + content;
    std::cout << text << '\n';
    }
}

bool Client::connect(const std::string& ip, uint16_t port)
{
    if(!tcpClient_->connect(ip,port))
    {
        return false;
    }
    connection_=tcpClient_->connection();
    std::cout<<"connection=" <<(connection_?"ok":"null")<<std::endl;

    if(!connection_)
    {
        return false;
    }
    connection_->setMessageCallback(std::bind(&Client::onMessage,this,std::placeholders::_1));
    connection_->setCloseCallback([this]() { onConnectionClosed(); });
    fileTransfer_ =std::make_unique<FileTransfer>(connection_);
    return true;
}

void Client::loginByPassword(const std::string& username, const std::string& password)
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

    msg.payload()["loginType"] = "password";
    msg.payload()["username"] = username;
    msg.payload()["password"] = password;

    connection_->send(msg);
}
void Client::sendLoginCode(const std::string& email)
{
    {
        std::lock_guard<std::mutex> lock(loginCodeMutex_);

        loginCodeFinished_ = false;
        loginCodeResult_ = false;
    }
    if(!connection_)
        return;
    Message msg;
    msg.setType(Messagetype::SendLoginCode);
    msg.payload()["email"] = email;
    connection_->send(msg);
}
void Client::loginByCode(const std::string& email,const std::string& code)
{
    {
        std::lock_guard<std::mutex> lock(loginMutex_);

        loginFinished_ = false;
        loginResult_ = false;
        login_ = false;
    }

    if(!connection_)
        return;
    Message msg;
    msg.setType(Messagetype::Login);
    msg.payload()["loginType"] = "code";
    msg.payload()["email"] = email;
    msg.payload()["code"] = code;

    connection_->send(msg);
}
void Client::sendRegisterCode(const std::string& email)
{
    {
        std::lock_guard<std::mutex> lock(registerCodeMutex_);

        registerCodeFinished_ = false;
        registerCodeResult_ = false;
    }

    if(!connection_)
        return;
    Message msg;
    msg.setType(Messagetype::SendRegisterCode);
    msg.payload()["email"] = email;
    connection_->send(msg);
}


void Client::sendResetCode(const std::string& email)
{
    {
        std::lock_guard<std::mutex> lock(resetCodeMutex_);

        resetCodeFinished_ = false;
        resetCodeResult_ = false;
    }

    if(!connection_)
        return;

    Message msg;
    msg.setType(Messagetype::SendResetCode);
    msg.payload()["email"] = email;

    connection_->send(msg);
}

void Client::resetPassword(const std::string& email,const std::string& code,const std::string& newPassword)
{
    {
        std::lock_guard<std::mutex> lock(resetPasswordMutex_);

        resetPasswordFinished_ = false;
        resetPasswordResult_ = false;
    }
    if(!connection_)
        return;
    Message msg;
    msg.setType(Messagetype::ResetPassword);
    msg.payload()["email"] = email;
    msg.payload()["code"] = code;
    msg.payload()["newPassword"] = newPassword;
    connection_->send(msg);
}
bool Client::waitingLoginResult()
{
    std::unique_lock<std::mutex> lock(loginMutex_);
    loginCv_.wait(lock, [this] {return loginFinished_; });
    loginFinished_ = false;
    return loginResult_;
}

bool Client::waitingLogincodeResult()
{
    std::unique_lock<std::mutex> lock(loginCodeMutex_);
    loginCodeCv_.wait(lock,[this]{return loginCodeFinished_;});
    loginCodeFinished_=false;
    return loginCodeResult_;

}
bool Client::waitRegisterResult()
{
    std::unique_lock<std::mutex> lock(registerMutex_);

    registerCv_.wait(lock, [this] {return registerFinished_;});

    registerFinished_ = false;

    return registerResult_;
}
bool Client::waitRegisterCodeResult()
{
    std::unique_lock<std::mutex> lock(registerCodeMutex_);

    registerCodeCv_.wait(lock, [this] {return registerCodeFinished_;});
    registerCodeFinished_ = false;
    return registerCodeResult_;
}

bool Client::waitResetCodeResult()
{
    std::unique_lock<std::mutex> lock(resetCodeMutex_);

    resetCodeCv_.wait(lock, [this] {
        return resetCodeFinished_;
    });

    resetCodeFinished_ = false;

    return resetCodeResult_;
}
bool Client::waitResetPasswordResult()
{
    std::unique_lock<std::mutex> lock(resetPasswordMutex_);

    resetPasswordCv_.wait(lock, [this] {
        return resetPasswordFinished_;
    });

    resetPasswordFinished_ = false;

    return resetPasswordResult_;
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

    if (isFriendBlock(receiverId))
    {
        std::cout << "该好友已被屏蔽，无法进入聊天"  << std::endl;
        return;
    }

    Message msg;
    msg.setType(Messagetype::PrivateChat);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setReceiverId(receiverId);
    msg.setTimestamp(time(nullptr));
    auto& payload=msg.payload();
    payload["content"]=text;
    connection_->send(msg);
      printChatMessage(userId_, userId_, "我", text);
}
void Client::createGroup(const std::string& groupName,const std::string&description,uint32_t friendId)
{
    Message msg;
    msg.setType(Messagetype::CreateGroup);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["groupName"]=groupName;
    msg.payload()["description"]=description;
    msg.payload()["friendId"]=friendId;
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
    msg.payload()["offline"] = false;
    auto& payload=msg.payload();
    payload["groupId"]=groupId;
    payload["content"]=text;
    connection_->send(msg);
    printChatMessage(userId_, userId_, "我", text); 
}
void Client::sendPrivateFile(uint32_t userId,const std::string& filename)
{
    if(!login_)
    return;
     if (isFriendBlockedEitherWay(userId))
    {
        std::cout << "双方有屏蔽，无法发送文件"  << std::endl;
        return;
    }
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
    {
        std::lock_guard<std::mutex> lock(friendListMutex_);
        friendListFinished_=false;
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
    disconnecting_ = true;        // 主动退出，不打印"服务器已关闭"

    if(tcpClient_)
    {
        tcpClient_->disconnect();
    }
    disconnecting_ = false;
    login_=false;
     userId_=0;
    username_.clear();
}
void Client::onConnectionClosed()
  {
      if (disconnecting_) return;
      login_ = false;               // 让 chatMenu 的 while(client.isLogin()) 退出

      std::cout << "\n[提示] 服务器已关闭连接\n" << std::endl;
      loop_->quit();
     
     std::cout.flush();

       ::_exit(0);
   
  }

void Client::quit()
{
    loop_->quit();
}
void Client::privateHistory(uint32_t userid,const std::string& username)
{
    if(!connection_)
        return;

     if (isFriendBlock(userid))
    {
        std::cout << "该好友已被屏蔽，无法查看历史"  << std::endl;
        return;
    }
    currentHistoryPeerName_=username;
    Message msg;
    msg.setType(Messagetype::HistoryRequest);
    msg.payload()["type"]=1;
    msg.payload()["peerId"]=userid;

    connection_->send(msg);

}
void Client::groupHistory(uint32_t gid)
{

    if(!connection_)
        return;

    Message msg;
    msg.setType(Messagetype::HistoryRequest);
     msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.payload()["type"]=2;
    msg.payload()["groupId"]=gid;
    connection_->send(msg);
}
void Client::onMessage(const Message& msg)
{
   
   switch(msg.type())
   {
    case Messagetype::RegisterResponse: handleRegister(msg); break;
    case Messagetype::LoginResponse:handleLogin(msg);
    break;
    case Messagetype::SendLoginCodeResponse:
    {
            bool success = false;

            if(msg.payload().contains("success"))
            {
                success =msg.payload()["success"].get<bool>();
            }

            {
                std::lock_guard<std::mutex> lock(loginCodeMutex_);

                loginCodeResult_ = success;
                loginCodeFinished_ = true;
            }
            loginCodeCv_.notify_one();
            break;
        }
     case Messagetype::SendRegisterCodeResponse:
        {
            bool success = false;

            if(msg.payload().contains("success"))
            {
                success =msg.payload()["success"].get<bool>();
            }

            {
                std::lock_guard<std::mutex> lock(registerCodeMutex_);

                registerCodeResult_ = success;
                registerCodeFinished_ = true;
            }

            registerCodeCv_.notify_one();
            break;
        }

        case Messagetype::SendResetCodeResponse:
        {
            bool success = false;
            if(msg.payload().contains("success"))
            {
                success =msg.payload()["success"].get<bool>();
            }

            {
                std::lock_guard<std::mutex> lock(resetCodeMutex_);

                resetCodeResult_ = success;
                resetCodeFinished_ = true;
            }

            resetCodeCv_.notify_one();
            break;
        }

        case Messagetype::ResetPasswordResponse:
        {
            bool success = false;
            if(msg.payload().contains("success"))
            {
                success =msg.payload()["success"].get<bool>();
            }

            {
                std::lock_guard<std::mutex> lock(resetPasswordMutex_);

                resetPasswordResult_ = success;
                resetPasswordFinished_ = true;
            }

            resetPasswordCv_.notify_one();
            break;
        }

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


   // case Messagetype::FileStart:
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
       std::cout << "[发送失败] "<< payload.value("message", "") << std::endl;
        break;
    }
    
     if(payload.contains("fileId"))
    {
        if(fileTransfer_)
    {
        auto stage = msg.payload().value("stage","");
      if(stage=="start" ||stage=="chunk" ||stage=="finish")
      {
        fileTransfer_->handleAck(msg);
       
      }
     }
        break;
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
      auto & payload=msg.payload();
       uint32_t groupId = payload.value("groupId", 0);
      std::string sendername =payload.value("senderName", "用户" + std::to_string(msg.senderId()));
      if (groupId != 0)
    {
        printNotification("收到群" + std::to_string(groupId) + "的待接收文件");
    }
    else
    {
      printNotification("收到"+sendername+"的待接收文件");
    }
       handleOfflineFileNotify(msg);
       break;
    }
    case Messagetype::BlockFriendResponse:
    {
        const auto& payload=msg.payload();
        int code=payload.value("code",-1);
        if(code==0)
        {
          std::cout<<"已经屏蔽好友\n";
          uint32_t friendId=payload.value("friendId",0u);
         for(auto& friendUser:friends_)
         {
            if(friendUser.value("id",0u)==friendId)
            {
                friendUser["blocked"]=true;
                break;
            }
         }
        }
        else
        {
            std::cout<<"屏蔽好友失败: "<<payload.value("message","")<<'\n';
        }
       break; 
    }
    case Messagetype::UnblockFriendResponse:
    {
        const auto& payload=msg.payload();
        int code=payload.value("code",-1);
        if(code==0)
        {
          std::cout<<"已经取消屏蔽好友\n";
          uint32_t friendId=payload.value("friendId",0u);
         for(auto& friendUser:friends_)
         {
            if(friendUser.value("id",0u)==friendId)
            {
                friendUser["blocked"]=false;
                break;
            }
         }
        }
        else
        {
            std::cout<<"取消屏蔽好友失败: "<<payload.value("message","")<<'\n';
        }
        break;
    }

   case Messagetype::AcceptFriendResponse:
   case Messagetype::RejectFriendResponse:
   case Messagetype::FriendRequestListResponse:
   handleFriend(msg);
   break;

   case Messagetype::GroupMemberListResponse:
   {
    const auto& payload = msg.payload();

       int code = payload.value("code", -1);
    {
        std::lock_guard<std::mutex> lock(groupMemberMutex_);
        groupMembers_.clear();

        if(payload.contains("members") && payload["members"].is_array())
        {
            for(const auto& member : payload["members"])
            {
                groupMembers_.push_back(member);
            }
        }
        groupMemberFinished_ = true;
    }

     if(code != 0)
    {
        std::cout << "获取群成员失败："<< payload.value("message", "")<< '\n';
    }
    groupMemberCv_.notify_one();

    break;
   }
    case Messagetype::GroupJoinRequestListResponse:
     {
    const auto& payload = msg.payload();
      {
        std::lock_guard<std::mutex> lock(groupJoinRequestMutex_);

        groupJoinRequests_.clear();
        if(payload.contains("requests") &&
           payload["requests"].is_array())
        {
            for(const auto& request : payload["requests"])
            {
                groupJoinRequests_.push_back(request);
            }
        }
        groupJoinRequestFinished_ = true;
    }
    groupJoinRequestCv_.notify_one();
    break;
    }
    case Messagetype::AcceptGroupJoinRequestResponse:
   {
    const auto& payload = msg.payload();
    int code = payload.value("code", -1);
    if(code == 0)
    {
        std::cout << "已接受入群申请\n";
    }
    else
    {
        std::cout << "接受入群申请失败："<< payload.value("message", "") << '\n';
    }
    break;
   }
   case Messagetype::RejectGroupJoinRequestResponse:
  {
    const auto& payload = msg.payload();
    int code = payload.value("code", -1);
    if(code == 0)
    {
        std::cout << "已拒绝入群申请\n";
    }
    else
    {
        std::cout << "拒绝入群申请失败：" << payload.value("message", "")<< '\n';
    }
    break;
   }
   case Messagetype::SetGroupAdminResponse:
   {
    const auto& payload = msg.payload();
    int code = payload.value("code", -1);
    if(code == 0)
    {
        int role = payload.value("role", 0);

        if(role == static_cast<int>(GroupRole::Admin))
        {
            std::cout << "已设置为管理员\n";
        }
        else
        {
            std::cout << "已取消管理员\n";
        }
    }
    else
    {
        std::cout << "设置成员角色失败："<< payload.value("message", "") << '\n';
    }

    break;
    }
    case Messagetype::RemoveGroupMemberResponse:
   {
    const auto& payload = msg.payload();
    int code = payload.value("code", -1);
    if(code == 0)
    {
        std::cout << "已移除群成员\n";
    }
    else
    {
        std::cout << "移除群成员失败："<< payload.value("message", "")<< '\n';
    }
    break;
   }
   case Messagetype::DisbandGroupResponse:
     {
      const auto& payload = msg.payload();
      int code = payload.value("code", -1);
      if(code == 0)
      {
          std::cout << "群聊已解散\n";
      }
      else
      {
          std::cout << "解散群聊失败：" << payload.value("message", "") << '\n';
      }
      break;
     }

   case Messagetype::DeleteAccountResponse:
   {
    bool success = false;
    if(msg.payload().contains("success"))
    {
        success = msg.payload()["success"].get<bool>();
    }
    std::string reason;
    if(msg.payload().contains("reaseon"))
    {
        reason=msg.payload()["reason"].get<std::string>();
    }
    {
        std::lock_guard<std::mutex> lock(deleteAccountResultMutex_);
        deleteAccountResult_ = success;
        deleteAccountFinished_ = true;
    }
    waitDeleteAccountCv_.notify_all();
    break;
     }
      case Messagetype::GroupJoinRequestNotify:
    {
        auto & payload=msg.payload();
         printNotification(payload.value("message", "收到一条新的入群申请"));
    break;
    }
        case Messagetype::GroupMemberLeaveNotify:
      {
          auto & payload = msg.payload();
          std::string leaver = payload.value("leaverName", "有成员");
          std::string groupname = payload.value("groupName", "");
          printNotification(leaver + " 退出了群聊 " + groupname);
          break;
      }

    case Messagetype::FriendRequestNotify:
    {
        auto & payload=msg.payload();
        printNotification(payload.value("message","收到一条新的好友申请"));
        break;
    }
    case Messagetype::PrivateUnreadNotify:
    {
       auto & payload=msg.payload();
       uint32_t  friendid=payload.value("friendId",0);
       std::string username=payload.value("userName","");
       int unreadc=payload.value("unreadCount",0);
       if(unreadc<=0)
       break;
        ChatMode mode;                                                     
      uint32_t chatId;                                                   
     {                                                                  
         std::lock_guard<std::mutex> lock(chatMutex_);                  
          mode = chatMode_;                                              
         chatId = currentChatId_;                                       
     }                                                                  
      if(inChat())                
        break;      
        
        
         std::cout << "\n[通知] 你有 " << unreadc << " 条来自用户 "<< username  << " 的未读消息\n";
         break;
    }
    case Messagetype::GroupUnreadNotify:
    {
        auto & payload=msg.payload();
       uint32_t  friendid=payload.value("groupId",0);
       std::string groupname=payload.value("groupName","");
       int unreadc=payload.value("unreadCount",0);
        if(unreadc<=0)
       break;
       
        if(inChat())                
        break;      
        
         std::cout << "\n[通知] 你有 " << unreadc << " 条来自群聊 "<< groupname  << " 的未读消息\n";
         break;
    }
    default:
    std::cout<<"unknow message type"<<std::endl;
    break;

   }
}
void Client::handleRegister(const Message& msg)
{
    bool success=false;

    const auto& payload = msg.payload();
    if(payload.contains("code"))
    {
        success= payload["code"].get<int>()==0;
    }

    {
        std::lock_guard<std::mutex> lock(registerMutex_);

        registerResult_ = success;
        registerFinished_ = true;
    }
    registerCv_.notify_one();
    if(success)
    {
        std::cout << "注册成功！\n";
    }
    else
    {
        if(payload.contains("message"))
        {
            std::cout<< "注册失败：" << payload["message"].get<std::string>() << "\n";
        }
        else
        {
            std::cout << "注册失败\n";
        }
    }
}
void Client::handleLogin(const Message& msg)
{
     const auto& payload=msg.payload();
     bool success=payload.value("success",false);

     {
         std::lock_guard<std::mutex> lock(loginMutex_);
     
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

    {
        std::lock_guard<std::mutex> lock(loginMutex_);
        loginResult_=success;
            loginFinished_=true;

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
     if(msg.type() == Messagetype::PrivateChatResponse )//&&  !payload.value("unreadDone", false))
   {
    if(payload.contains("code"))
    {
        int code = payload.value("code", -1);
        if(code != 0)
        {
            std::cout <<"发送失败："<< payload.value( "message","无法进入聊天") << std::endl;
            leaveChat();
            return;
        }
    }
    {
        // 服务器已经响应
       
            std::lock_guard<std::mutex> lock(chatMutex_);
            chatMode_ = ChatMode::Private;
        
    }
    }
     uint32_t senderId = msg.senderId();
     uint32_t receiverId =msg.receiverId();
    
     uint32_t peerId = 0;
     if(senderId == userId_)
    {
    peerId = receiverId;
    }
    else
    {
    peerId = senderId;
     }

      if(payload.value("unreadDone", false))
    {
       

        PrivateChatRead(peerId);
        clearLocalUnreadCount(peerId);

        return;
    }
   std::string content=payload.value("content","");
    bool unread =payload.value("unread", false);
    bool realtime = payload.value("realtime", false);
    ChatMode mode;
    uint32_t chatId;
    std::string peerName;

    {
        std::lock_guard<std::mutex> lock(chatMutex_);

        mode = chatMode_;
        chatId = currentChatId_;
        peerName = currentChatPeerName_;
    }
   
    if(mode == ChatMode::Private && (chatId == senderId|| receiverId==chatId))
    {
        
        {
            std::lock_guard<std::mutex> lock(chatOutputMutex_);
         std::cout << '\n';
         std::string senderName;

        if(senderId == userId_)
        {
            senderName = "我";
        }
        else
        {
            senderName =payload.value("senderName",peerName);
             if(senderName.empty())
            {
                senderName = peerName;
            }

            if(senderName.empty())
            {
                senderName = "用户" + std::to_string(senderId);
            }
        }
       printChatMessage(senderId,userId_,senderName,content);
        }
    

       return;
    }
    if(realtime)
    {
        std::string senderName = payload.value("senderName", "用户" + std::to_string(senderId));
     printNotification("收到 " + senderName + " 的新消息");
    }
      

    return;
}
void Client::handleGroupChat(const Message& msg)
{
   
    const auto& payload=msg.payload();

    uint32_t groupId=payload.value("groupId",0u);
    uint32_t senderId = msg.senderId();

    
    std::string content=payload.value("content","");
    bool offline=payload.value("offline",false);
    std::string senderName =payload.value("senderName","用户" + std::to_string(senderId));

    ChatMode mode;
    uint32_t currentGroupId;

    {
        std::lock_guard<std::mutex> lock(chatMutex_);
        mode = chatMode_;
        currentGroupId = currentChatId_;
    }
    if(mode == ChatMode::Group && currentGroupId == groupId)
    {
       std::lock_guard<std::mutex> lock(chatOutputMutex_);
         std::cout << '\n';
        printChatMessage(senderId, userId_, senderName, content);
        return;

    }

    if(offline)//这个offline实际就是unread
    {
        return;
    }
    
      printNotification("收到群 " + std::to_string(groupId) + " 的新消息");
        return;
  
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
         printNotification("添加好友申请发送" );
       
    } 
    else
    {
        
         printNotification("添加好友失败" );
        std::cout<<payload.value("message","")<<std::endl;
    }
    break;
    }
    case Messagetype::DeleteFriendResponse:
    {
     int code=payload.value("code",-1);
      if(code==0)
      {
       
         printNotification("删除好友成功" );
      }
      else
      {
        std::cout<<"删除好友失败"<<std::endl;
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
     {
        std::lock_guard<std::mutex> lock(friendListMutex_);
        friendListFinished_=true;
     }
     friendListCv_.notify_one();
    if(friends_.empty())
    {
        std::cout << "你还没有好友" << std::endl;
        break;
    }
    std::cout << "\n========== 好友列表 ==========\n";
    for(const auto& friendUser : friends_)
    {
        
     bool online = friendUser.value("online", false);
     std::cout<< "id: " << friendUser.value("id", 0)<< "\nusername: " << friendUser.value("username", "");
     bool blocked =friendUser.value("blocked", false);
     bool blockedByFriend =friendUser.value("blockedByFriend", false);

     if(blocked)
    {
    std::cout << " 【我已屏蔽】\n";
    }

    if(blockedByFriend)
    {
    std::cout << " 【对方已屏蔽我】\n";
    }
        std::cout<<"\nnickname: " << friendUser.value("nickname", "")<< "\nstatus: " << friendUser.value("status", 0)
        << "\nonline: " << (online ? "在线" : "离线")<< '\n';
        uint32_t friendId = friendUser.value("id", 0);

        size_t unreadCount = friendUser.value("unreadCount",0);
        if(unreadCount > 0)
        {
            std::cout << "未读消息: "<< unreadCount << "条\n";
        }
      
         std::cout<< "\n-----------------------------\n";
    }
    std::cout << "==============================\n";
    std::cout << "\n0. 返回\n";
    std::cout << "请选择好友ID：" << std::flush;
    break;
    }
    
    case Messagetype::FriendRequestListResponse:
   {

    if(!payload.contains("requests") || !payload["requests"].is_array())
    {
        std::cout << "好友申请列表为空\n";

        {
            std::lock_guard<std::mutex> lock(friendRequestMutex_);
            friendRequests_ = nlohmann::json::array();
            friendRequestFinished_ = true;
        }

        friendRequestCv_.notify_one();

        break;
    }

    friendRequests_ = payload["requests"];

    {
        std::lock_guard<std::mutex> lock(friendRequestMutex_);
        friendRequestFinished_ = true;
    }
    friendRequestCv_.notify_one();

    break;
   }
   case Messagetype::AcceptFriendResponse:
  {
    int code = payload.value("code", -1);

    if(code == 0)
    {
        std::cout << "已同意好友申请\n";
    }
    else
    {
        std::cout<< "同意好友申请失败: "<< payload.value("message", "")<< '\n';
    }

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
           
          printNotification("创建群成功" );
            std::cout << "【groupId】="<< payload.value("groupId", 0)<< std::endl;
        }
        else
        {
            std::cout << "create group failed: "<< message<< std::endl;
        }
        break;

    case Messagetype::JoinGroupResponse:
        if(code == 0)
        {
           
         printNotification("入群申请发送" );
        }
        else
        {
           
         printNotification("入群失败" );
        }
        break;

    case Messagetype::LeaveGroupResponse:
        if(code == 0)
        {
          
         printNotification("退出群聊成功" );
        }
        else
        {
            std::cout << "退群失败： "<< message<< std::endl;
        }
        break;

    case Messagetype::GroupChatResponse:
        if(code == 0)
        {
        }
        else
        {
            std::cout << "群消息发送失败 "<< message<< std::endl;
        }
        break;

    default:
        break;
    }
}

void Client::handleFile(const Message& msg)
{
    switch(msg.type())
    {
    
        case Messagetype::FileChunk:
          fileTransfer_->handleFileChunk(msg);
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

void Client::handleHistory(const Message& msg)
{
    auto& payload=msg.payload();
     int code = payload.value("code", -1);
    
    if(code != 0)
    {
        std::cout << "无法查看历史记录 " << payload.value("message", "") << std::endl;
        return;
    }
    if(!payload.contains("message"))
    {
        
         printNotification("历史记录为空" );
        return;
    }
    auto messages=payload["message"];

    if(!messages.is_array())
    {
        std::cout<<"messages not array\n";
        return;
    }
    std::reverse(messages.begin(),messages.end());
    int historyType=payload.value("type",1);

    std::cout<<"======== 历史消息 ========\n";
    for(auto&it:messages)
    {

        uint32_t senderId=it["senderId"];
        std::string content=it["content"];
        std::string senderName;
        if(senderId==userId_)
        {
            senderName="我";
        }
        else if(historyType==1)
        {
            senderName=currentHistoryPeerName_;
            if(senderName.empty())
            {
              senderName="用户"+std::to_string(senderId);
            }
        }
        else
        {
            senderName=it.value("senderName","用户"+std::to_string(senderId));
        }
        printChatMessage(senderId,userId_,senderName,content);
        
    }

    std::cout<<"==========================\n";

}

void Client::handleOfflineFileNotify(const Message& msg)
{

  fileTransfer_->handleOfflineFileNotify(msg);
}
const std::vector<FileTransfer::PendingReceiveFile> Client::pendingReceiveFiles() const
{
    return fileTransfer_->pendingReceiveFiles();
}

bool Client::acceptFile(int64_t fileId)
{
    if(!fileTransfer_)
        return false;
    fileTransfer_->acceptFile(fileId);
    return true;
}
void Client::registerUser(const std::string& username,const std::string& password, const std::string& email,const std::string& code)
{
    if(!connection_)
    {
        std::cout << "服务器连接不存在\n";
        return;
    }

    Message msg;
    msg.setType(Messagetype::Register);
    msg.setSequence(sequence_++);
    msg.payload()["username"] = username;
    msg.payload()["password"] = password;
    msg.payload()["email"] = email;
    msg.payload()["code"] = code;

    {
        std::lock_guard<std::mutex> lock(registerMutex_);

        registerFinished_ = false;
        registerResult_ = false;
    }

    connection_->send(msg);
}
void Client::enterPrivateChat(uint32_t friendid,const std::string& friendname)
{

    if(!connection_ || !login_)
        return;
    {std::lock_guard<std::mutex> lock(chatMutex_);

    currentChatId_ = friendid;
     currentChatPeerName_ = friendname;
    }
   
    Message msg;
    msg.setType(Messagetype::PrivateUnreadRequest);
     msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setReceiverId(friendid);
    msg.payload()["peerId"] = friendid;

    connection_->send(msg);
    
}
void Client::enterGroupChat(uint32_t groupid)
{
     {
        std::lock_guard<std::mutex> lock(chatMutex_);
        chatMode_ = ChatMode::Group;
        currentChatId_ = groupid;
    }

    Message msg;
    msg.setType(Messagetype::GroupChatRead);
    msg.setReceiverId(0);
    msg.payload()["groupId"] = groupid;

    connection_->send(msg);
}
void Client::PrivateChatRead(uint32_t friendid)
{
    if(!connection_ || !login_)
        return;

    Message msg;

    msg.setType(Messagetype::PrivateChatRead);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setReceiverId(friendid);
    msg.setTimestamp(time(nullptr));
    
       msg.payload()["peerId"] = friendid;
    connection_->send(msg);
}
void Client::clearLocalUnreadCount(uint32_t friendid)
{
    for(auto& friendUser : friends_)
    {
        if(friendUser.value("id", 0u) == friendid)
        {
            friendUser["unreadCount"] = 0;
            break;
        }
    }
}
void Client::leaveChat()
{
    std::lock_guard<std::mutex> lock(chatMutex_);

    chatMode_ = ChatMode::None;
    currentChatId_ = 0;
     currentChatPeerName_.clear();
}

bool Client::inChat() const
{
    std::lock_guard<std::mutex> lock(chatMutex_);

    return chatMode_ != ChatMode::None;
}

Client::ChatMode Client::chatMode() const
{
    std::lock_guard<std::mutex> lock(chatMutex_);

    return chatMode_;
}

void Client::blockFriend(uint32_t friendId)
{
    if(!connection_ ||! login_)
    {
        return;
    }
    if(friendId==userId_)
    {
        std::cout<<"不能屏蔽自己\n";
        return;
    }
    if(isFriendBlock(friendId))
    {
        std::cout<<"该好友已经被屏蔽\n";
        return;
    }
    Message msg;
    msg.setType(Messagetype::BlockFriend);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["friendId"]=friendId;
    connection_->send(msg);

}
void Client::unblockFriend(uint32_t friendId)
{

    if(!connection_ ||! login_)
    {
        return;
    }
    if(!isFriendBlock(friendId))
    {
        std::cout<<"该好友当前没有被屏蔽\n";
        return;
    }

     Message msg;
    msg.setType(Messagetype::UnblockFriend);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["friendId"]=friendId;
    connection_->send(msg);
  
    
}
bool Client::isFriendBlock(uint32_t friendId) const
{
    for(const auto& friendUser : friends_)
    {
        if(friendUser.value("id", 0u) == friendId)
        {
            return friendUser.value("blocked", false);
        }
    }

    return false;
}
bool Client::isFriendBlockedEitherWay(uint32_t friendId) const
{
    for(const auto& friendUser : friends_)
    {
        if(friendUser.value("id", 0u) == friendId)
        {
            bool blocked = friendUser.value("blocked", false);
            bool blockedByFriend = friendUser.value("blockedByFriend", false);

            return blocked || blockedByFriend;
        }
    }

    return false;
}
void Client::waitFriendList()
{
    std::unique_lock<std::mutex> lock(friendListMutex_);

    friendListCv_.wait(
        lock,
        [this]
        {
            return friendListFinished_;
        });

    friendListFinished_ = false;
}
void Client::friendRequestList()
{
    if(!connection_ || !login_)
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(friendRequestMutex_);

        friendRequestFinished_ = false;
    }

    Message msg;
    msg.setType(Messagetype::FriendRequestList);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    connection_->send(msg);
}

bool Client::waitFriendRequestList()
{
    std::unique_lock<std::mutex> lock(friendRequestMutex_);
    friendRequestCv_.wait(lock, [this] { return friendRequestFinished_; });
    friendRequestFinished_ = false;
    return true;
}
void Client::acceptFriend(uint32_t requestId)
{
    if(!connection_ || !login_)
    {
        return;
    }

    Message msg;
    msg.setType(Messagetype::AcceptFriend);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["requestId"] = requestId;

    connection_->send(msg);
}
void Client::rejectFriend(uint32_t requestId)
{
    if(!connection_ || !login_)
    {
        return;
    }

    Message msg;
    msg.setType(Messagetype::RejectFriend);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
     msg.payload()["requestId"] = requestId;

    connection_->send(msg);
}
void Client::groupJoinRequestList(int64_t groupId)
{
    if(!connection_ || !login_)
    {
        return;
    }
    Message msg;
    msg.setType(Messagetype::GroupJoinRequestList);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["groupId"] = groupId;
    connection_->send(msg);
}
void Client::acceptGroupJoinRequest(int64_t requestId)
{
    if(!connection_ || !login_)
    {
        return;
    }

    Message msg;
    msg.setType(Messagetype::AcceptGroupJoinRequest);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["requestId"] = requestId;
    connection_->send(msg);
}
void Client::rejectGroupJoinRequest(int64_t requestId)
{
    if(!connection_ || !login_)
    {
        return;
    }
    Message msg;
    msg.setType(Messagetype::RejectGroupJoinRequest);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["requestId"] = requestId;
    connection_->send(msg);
}
void Client::setGroupMemberRole(int64_t groupId,int userId, GroupRole role)
{
    if(!connection_ || !login_)
    {
        return;
    }
    Message msg;
    msg.setType(Messagetype::SetGroupAdmin);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["groupId"] = groupId;
    msg.payload()["userId"] = userId;
    msg.payload()["role"] = static_cast<int>(role);
    connection_->send(msg);
}
void Client::removeGroupMember(int64_t groupId,int userId)
{
    if(!connection_ || !login_)
    {
        return;
    }

    Message msg;
    msg.setType(Messagetype::RemoveGroupMember);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["groupId"] = groupId;
    msg.payload()["userId"] = userId;
    connection_->send(msg);
}
  void Client::disbandGroup(int64_t groupId)
  {
      if(!connection_ || !login_)
      {
          return;
      }
      Message msg;
      msg.setType(Messagetype::DisbandGroup);
      msg.setSequence(sequence_++);
      msg.setSenderId(userId_);
      msg.setTimestamp(time(nullptr));
      msg.payload()["groupId"] = groupId;
      connection_->send(msg);
  }

bool Client::waitGroupMemberList()
{
    std::unique_lock<std::mutex> lock(groupMemberMutex_);
    groupMemberCv_.wait(lock,[this] { return groupMemberFinished_;});
    groupMemberFinished_ = false;
    return true;
}
bool Client::waitGroupJoinRequestList()
{
    std::unique_lock<std::mutex> lock(groupJoinRequestMutex_);
    groupJoinRequestCv_.wait(lock,[this] {return groupJoinRequestFinished_;});
    groupJoinRequestFinished_ = false;
    return true;
}
void Client::groupMemberList(std::int64_t groupId)
{
    if(!connection_ || !login_)
    {
        return ;
    }
    Message msg;
    msg.setType(Messagetype::GroupMemberList);
    msg.setSequence(sequence_++);
    msg.setSenderId(userId_);
    msg.setTimestamp(time(nullptr));
    msg.payload()["groupId"] = groupId;
    connection_->send(msg);
    return ;
}
void Client::deleteAccount()
{
    {
        std::lock_guard<std::mutex> lock(deleteAccountResultMutex_);
        deleteAccountFinished_ = false;
        deleteAccountResult_ = false;
    }
    Message msg;
    msg.setType(Messagetype::DeleteAccount);
    msg.setSenderId(userId_);
    connection_->send(msg);
}
bool Client::waitDeleteAccountResult()
{
    std::unique_lock<std::mutex> lock(deleteAccountResultMutex_);
    waitDeleteAccountCv_.wait(lock, [this] { return deleteAccountFinished_;});
    return deleteAccountResult_;
}

std::vector<FileTransfer::DownloadProgress> Client::downloadProgressList()const{
    if(!fileTransfer_)
    {
        return {};
    }
    return fileTransfer_->downloadProgressList();
}