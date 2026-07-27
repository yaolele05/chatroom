#include "usersession.h"
#include <chrono>
UserSession::UserSession(const TcpConnectionptr& conn):Session(conn),lastHeartbeat_(std::chrono::steady_clock::now())
{
       
}
void UserSession::login(int userid,const std::string& username)
{
    userid_=userid;
    username_=username;
    online_=true;
    authenticated_=true;
    updateHeartbeat();
}
void UserSession::logout()
{
    reset();
}
void UserSession::reset()
{
    userid_=0;
    username_.clear();
    online_=false;
    authenticated_=false;
    clientIp_.clear();
    clientPort_=0;
    lastHeartbeat_=std::chrono::steady_clock::now();

}

int UserSession::userid() const
{
    return userid_;
}
const std::string& UserSession::username()const
{
    return username_;
}
bool UserSession::online() const
{
    return online_;
}
bool UserSession::authenticated() const
{
    return  authenticated_; 
}

void UserSession::setOnline(bool online)
{
    online_=online;
}
void UserSession::setAuthenticated(bool v)
{
    authenticated_=v;
}
void UserSession:: setUsername(const std::string& username)
{
    username_=username;
}

void UserSession::updateHeartbeat()
{
    lastHeartbeat_=std::chrono::steady_clock::now();
}
void UserSession::setClientAddress(const std::string& ip,uint16_t port)
{
    clientIp_=ip;
    clientPort_=port;
}
uint16_t UserSession::clientPort() const
{
    return clientPort_;
}
const std::string& UserSession::clientIp() const
{
    return clientIp_;
}
std::chrono::steady_clock::time_point UserSession::lastHeartbeat() const
{
    return lastHeartbeat_;
}

