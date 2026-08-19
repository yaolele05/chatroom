#pragma once
#include <condition_variable>
#include <mutex>
#include "tcpclient.h"
#include "../common/protocol/message.h"
#include "../server/model/groupmodel.h"
#include <memory>
#include <string>
#include "clientconnection.h"
#include "filetransf.h"
#include <unordered_map>
#include <vector>
#include <unordered_set>
class EventLoop;
class TcpClient;
class FileTransfer;

class Client
{
    public:
   

    enum class ChatMode
    {
    None,
    Private,
    Group
    };
    explicit Client(EventLoop* loop);
     ~Client();

    bool connect(const std::string&ip,uint16_t port);
    void printChatMessage( uint32_t senderId,uint32_t currentUserId,const std::string& senderName,const std::string& content);

    void loginByPassword(const std::string& username,const std::string& password);
    void loginByCode(const std::string& email, const std::string& code);
    void sendLoginCode(const std::string& email);
    bool waitingLoginResult();
    bool waitingLogincodeResult();
    void sendRegisterCode(const std::string& email);
    void registerUser(const std::string& username,const std::string& password, const std::string& email,const std::string& code);
    bool waitRegisterCodeResult();
    bool waitRegisterResult();
    void sendResetCode(const std::string& email);
    void resetPassword(const std::string& email,const std::string& code,const std::string& newPassword);
    bool waitResetCodeResult();
    bool waitResetPasswordResult();

        void logout();
    void addFriend(uint32_t friendId);
    void deleteFriend(uint32_t friendId);
     void friendList();
    void privateChat(uint32_t userId,const std::string& text);
    void createGroup(const std::string& groupName,const std::string& description);
    void joinGroup(uint32_t groupId);
    void leaveGroup(uint32_t groupId);
    void groupChat(uint32_t groupId,const std::string& text);
     void groupList();
    void sendPrivateFile(uint32_t userId,const std::string& filename);
    void sendGroupFile(uint32_t groupId,const std::string& filename);
    void sendImage(uint32_t userId,const std::string& filename);
    bool isLogin() const;
   
   
     void disconnect();
     void quit();
    
     void privateHistory(uint32_t userid,const std::string& username);
     void groupHistory(uint32_t groupid);
     const std::vector<FileTransfer::PendingReceiveFile>&  pendingReceiveFiles() const;
     //bool rejectFile(int64_t fileId);
     void acceptFile(int64_t fileId);
     //showFileNotification();
  
    const nlohmann::json& friends() const
    {
        return friends_;
    }
    const nlohmann::json& groups() const
    {
        return groups_;
    }
    uint32_t userId() const
    { 
    return userId_;
    } 

    void enterPrivateChat(uint32_t friendid,const std::string& friendname);
    void enterGroupChat(uint32_t groupid);
    void leaveChat();

    bool inChat() const;
    ChatMode chatMode() const;
    uint32_t currentChatId() const;
  
    void PrivateChatRead(uint32_t friendid);
    void clearLocalUnreadCount(uint32_t friendid);
   void requestPrivateUnread(uint32_t friendid);
     void blockFriend(uint32_t friendId);
    void unblockFriend(uint32_t friendId);
    bool isFriendBlock(uint32_t friendId) const;
    const std::unordered_set<uint32_t>& blockedFriends() const
    {
        return blockedFriends_;
    }
    bool isFriendBlockedEitherWay(uint32_t friendId) const;
    void waitFriendList();
 
     void   friendRequestList();
        bool waitFriendRequestList();
    const nlohmann::json& friendRequests() const
    {
    return friendRequests_;
    }
     void acceptFriend(uint32_t userid);
     void rejectFriend(uint32_t userid);
   void groupMemberList(int64_t groupId);
   void groupJoinRequestList(int64_t groupId);
   void acceptGroupJoinRequest(int64_t requestId);
   void rejectGroupJoinRequest(int64_t requestId);
   void setGroupMemberRole(int64_t groupId,int userId,GroupRole role);
   void removeGroupMember(int64_t groupId,int userId);

   bool waitGroupMemberList();
   bool waitGroupJoinRequestList();
   
   const std::vector<nlohmann::json>& groupMembers() const
   {
    return groupMembers_;
   }

   const std::vector<nlohmann::json>& groupJoinRequests() const
   {
    return groupJoinRequests_;
   }

   void deleteAccount();
  bool waitDeleteAccountResult();


    private:
    void onMessage(const Message& message);
    void handleLogin(const Message& msg);
    void handleLogout(const Message& msg);
    void handlePrivateChat(const Message& msg);
    void handleFriend(const Message& msg);
    void handleGroup(const Message& msg);
    void handleGroupChat(const Message& msg);
    void handleGroupList(const Message& msg);
    void handleFile(const Message& msg);
    void handleHeartbeat(const Message& msg);
    void handleRegister(const Message& msg);
    void handleHistory(const Message& msg);
    void handlerequestDownload(int64_t fileId);
    void handleOfflineFileNotify(const Message& msg);
    

   private:
   EventLoop* loop_;
   std::unique_ptr<TcpClient> tcpClient_;
   std::unique_ptr<FileTransfer> fileTransfer_;
   std::shared_ptr<ClientConnection> connection_;

    std::string username_;
   uint32_t userId_{0};
    uint64_t sequence_{1};
   bool login_{false};
    bool loginFinished_{false};
    bool loginResult_{false};
    std::mutex loginMutex_;
    std::condition_variable loginCv_;
    bool registerCodeFinished_{false};
    bool registerCodeResult_{false};
    bool registerFinished_{false};
    bool registerResult_{false};
    std::mutex registerCodeMutex_;
    std::condition_variable  registerCodeCv_ ;
    std::mutex registerMutex_;
    std::condition_variable registerCv_;
    bool loginCodeFinished_{false};
    bool loginCodeResult_{false};
    std::mutex loginCodeMutex_;
    std::condition_variable loginCodeCv_;

    bool resetCodeFinished_{false};
    bool resetCodeResult_{false};
    std::mutex resetCodeMutex_;
    std::condition_variable resetCodeCv_;
    bool resetPasswordFinished_{false};
    bool resetPasswordResult_{false};
     std::mutex resetPasswordMutex_;
    std::condition_variable resetPasswordCv_;

    nlohmann::json friends_;
    nlohmann::json groups_;
    std::string currentHistoryPeerName_;
    std::string currentChatPeerName_;

    mutable std::mutex chatMutex_;
    ChatMode chatMode_ = ChatMode::None;
    uint32_t currentChatId_ = 0;
    mutable std::mutex chatOutputMutex_;
   
    std::unordered_set<uint32_t> blockedFriends_;
    std::mutex friendListMutex_;
   std::condition_variable friendListCv_;
   bool friendListFinished_ = false;

       std::mutex friendRequestMutex_;
    std::condition_variable friendRequestCv_;
   bool friendRequestFinished_ = false;
   nlohmann::json friendRequests_;

   std::vector<nlohmann::json> groupMembers_;
   std::vector<nlohmann::json> groupJoinRequests_;
   std::mutex groupMemberMutex_;
  std::condition_variable groupMemberCv_;
   bool groupMemberFinished_ = false;

   std::mutex groupJoinRequestMutex_;
     std::condition_variable groupJoinRequestCv_;
    bool groupJoinRequestFinished_ = false;

    std::mutex deleteAccountResultMutex_;
     std::condition_variable waitDeleteAccountCv_;
     bool deleteAccountFinished_ = false;
    bool deleteAccountResult_ = false;
   
};