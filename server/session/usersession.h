#pragma once
#include "session.h"
#include <string>
#include <chrono>
#include "../../minimuduo/net/callback.h"
#include <mutex>
class UserSession:public Session
{
    public:
     explicit UserSession(const TcpConnectionptr& conn=nullptr);

     void login(int userid,const std::string& name);
     void logout();
     void reset();

     int userid() const override;
     const std::string&username() const;

     bool online() const;
     bool authenticated() const override;
     void setUserid(int id);
     void setOnline(bool online);
     void setAuthenticated(bool v);
     void setUsername(const std::string& username);
     void updateHeartbeat();
     std::chrono::steady_clock::time_point lastHeartbeat() const;
     bool heartbeatTimeout(std::chrono::seconds timeout) const;///

     void setClientAddress(const std::string& ip,uint16_t port);
     uint16_t clientPort() const;
     const std::string& clientIp() const;
     void updateActivity()
    {
         std::lock_guard<std::mutex> lock(activityMutex_);
    lastActivity_ = std::chrono::steady_clock::now();
     }
     std::chrono::steady_clock::time_point lastActivity() const
    {
    return lastActivity_;
    }

     private:
     int userid_{0};
     std::string username_;
     bool online_{false};
     bool authenticated_{false};
     std::chrono::steady_clock::time_point lastHeartbeat_;
     std::chrono::steady_clock::time_point lastActivity_;
     std::string clientIp_;
     uint16_t clientPort_{0};
     mutable std::mutex activityMutex_;
};