#include "sessionmanager.h"
#include <memory>
#include <unordered_map>
#include <mutex>
SessionManager&  SessionManager::instance()
{
    static SessionManager manager;
    return manager;
}
bool SessionManager:: addSession(const std::shared_ptr<UserSession>& session)
{
   if(!session)
   {
    return false;
   }
   std::lock_guard<std::mutex> lock(mutex_);
   useridsessions_[session->userid()]=session;
   usernamesessions_[session->username()]=session;
   connectionsessions_[session->connection().get()]=session;

   return true;
}
bool SessionManager::removeSession(TcpConnection* conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it =connectionsessions_.find(conn);
      if(it==connectionsessions_.end())
      return false;

      auto session=it->second;
      useridsessions_.erase(session->userid());
       usernamesessions_.erase(session->username());
      connectionsessions_.erase(it);////

    return true;
}
SessionManager::UserSessionPtr SessionManager::getSession(const int userid)
{
    std::lock_guard<std::mutex> lock(mutex_);
     auto it=useridsessions_.find(userid);
     if(it==useridsessions_.end())
     return nullptr;

     return it->second;
}
SessionManager::UserSessionPtr SessionManager::getSession(const std::string& username)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it =usernamesessions_.find(username);
    if(it==usernamesessions_.end())
    return nullptr;

    return it->second;
}
SessionManager::UserSessionPtr SessionManager::getSession( TcpConnection* conn)
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto it=connectionsessions_.find(conn);
    if(it==connectionsessions_.end())
    return nullptr;

    return it->second;
}
bool SessionManager:: contains(int userid) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return useridsessions_.find(userid)!=useridsessions_.end();
}
bool SessionManager::contains(const std::string& username)
{
    std::lock_guard<std::mutex> lock(mutex_);
    return usernamesessions_.find(username)!=usernamesessions_.end();
}
size_t SessionManager:: onlineCount() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return useridsessions_.size();
}
std::vector<SessionManager::UserSessionPtr> SessionManager::onlineUsers() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<UserSessionPtr> users;
    users.reserve(useridsessions_.size());

    for (const auto& [id, session] : useridsessions_)
    {
        users.push_back(session);
    }

    return users;
}
void SessionManager::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    useridsessions_.clear();
    usernamesessions_.clear();
    connectionsessions_.clear();
}
void SessionManager::foreachSession(const std::function<void(SessionManager::UserSessionPtr)>& cb)
{
   std::lock_guard<std::mutex> lock(mutex_);
   
    for(auto& [id,session] : useridsessions_)
  {
    cb(session);
  }
}