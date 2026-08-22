#pragma once
#include "../../session/usersession.h"
#include "../../model/offlinemodel.h"
class OfflineService
{
    public:
    static OfflineService& instance();
    void sendOfflineMessage(Session* session);//登录是统一入口
    void   sendPrivateUnreadNotify(int userId, Session* se);
   void sendGroupUnreadNotify(int userId, Session* se); 
    void sendOfflineFriendRequest( Session* se); 
   void sendPrivateOfflineMessages(int userId, int friendId, Session* se);
    void sendGroupOfflineMessages(int userId, int64_t groupId, Session* se);
    private:
    OfflineService()=default;
    OfflineService(const OfflineService&)=delete;
    OfflineService& operator=(const OfflineService&)=delete;

    bool sendChatOffline(const OfflineMessage&offline,Session* se);
    void sendGroupOffline(const OfflineMessage& offline,Session* se);
    void sendOfflineFile( Session* se);

};