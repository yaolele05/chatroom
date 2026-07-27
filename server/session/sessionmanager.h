#pragma once
#include <unordered_map>
#include <memory>
#include <mutex>
#include "usersession.h"
#include "Tcpconnection.h"
class SessionManager
{
    public:

    using UserSessionPtr=std::shared_ptr<UserSession>;
    static SessionManager& instance();

    bool addSession(const UserSessionPtr& session);
    bool removeSession(TcpConnection* conn);

    UserSessionPtr getSession(const int userid);
    UserSessionPtr getSession(const std::string& username);
    UserSessionPtr getSession( TcpConnection* conn);
    bool contains(int userid) const;
    bool contains(const std::string&);
    size_t onlineCount() const;
    std::vector<UserSessionPtr> onlineUsers() const;///
    void clear();
    void foreachSession(const std::function<void(UserSessionPtr)>& cb);///

    private:
    SessionManager()=default;
    ~SessionManager()=default;
    SessionManager(const SessionManager&)=delete;
    SessionManager& operator=(const SessionManager& )=delete;
   
    std::unordered_map<int,UserSessionPtr> useridsessions_;
    std::unordered_map<std::string,UserSessionPtr > usernamesessions_;
     std::unordered_map<TcpConnection*,UserSessionPtr> connectionsessions_;                         

    mutable std::mutex mutex_;
};