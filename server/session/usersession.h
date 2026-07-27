#pragma once
#include "session.h"
#include <string>
#include <chrono>
#include "callback.h"
class UserSession:public Session
{
    public:
     explicit UserSession(const TcpConnectionptr& conn);

     void login(int userid,const std::string& name);
     void logout();
     void reset();

     int userid() const;
     const std::string&username() const;

     bool online() const;
     bool authenticated() const;
     void setOnline(bool online);
     void setAuthenticated(bool v);
     void setUsername(const std::string& username);


     void updateHeartbeat();
     std::chrono::steady_clock::time_point lastHeartbeat() const;
     bool heartbeatTimeout(std::chrono::seconds timeout) const;///

     void setClientAddress(const std::string& ip,uint16_t port);
     uint16_t clientPort() const;
     const std::string& clientIp() const;
     
    

     private:
     int userid_{0};
     std::string username_;
     bool online_{false};
     bool authenticated_{false};
     std::chrono::steady_clock::time_point lastHeartbeat_;
     std::string clientIp_;
     uint16_t clientPort_{0};
};