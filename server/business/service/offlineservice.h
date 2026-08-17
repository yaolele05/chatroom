#pragma once
#include "../../session/usersession.h"
#include "../../model/offlinemodel.h"
class OfflineService
{
    public:
    static OfflineService& instance();
    void sendOfflineMessage(Session* session);
    void sendOfflineFile( Session* se);
    private:
    OfflineService()=default;
    OfflineService(const OfflineService&)=delete;
    OfflineService& operator=(const OfflineService&)=delete;

    bool sendChatOffline(const OfflineMessage&offline,Session* se);
    void sendGroupOffline(const OfflineMessage& offline,Session* se);
    
    
 

};