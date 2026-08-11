#pragma once
#include <condition_variable>
#include <mutex>
#include "tcpclient.h"
#include "../common/protocol/message.h"
#include <memory>
#include <string>
#include "clientconnection.h"

class EventLoop;
class TcpClient;
class FileTransfer;

class Client
{
    public:
    explicit Client(EventLoop* loop);
     ~Client();

    bool connect(const std::string&ip,uint16_t port);
    void login(const std::string& username,const std::string& password);
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
    void registerUser(const std::string& username,const std::string& password);
   
     void disconnect();
     void quit();
     bool waitLoginResult();
     void privateHistory(uint32_t userid);
     void groupHistory(uint32_t groupid);
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
    void handleOfflineFileNotify(const Message& msg);
    void handlerequestDownload(int64_t fileId);
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
   std::mutex loginMutex_;
std::condition_variable loginCv_;
};