#pragma once
#include "Tcpconnection.h"
#include "callback.h"
#include <memory>////
#include <string>
#include <unordered_map>
#include "noncopyable.h"

class EventLoop;
class Acceptor;

class InetAddress;
class EventLoopThreadPool;

class TcpServer:public noncopyable
{
  public:
    


    TcpServer(EventLoop* loop,const InetAddress& listenAddr,const std::string& name);
    ~TcpServer();
    
    void start();
    void setThreadNum(int numThreads);
    void setConnectionCallback( const Connectioncallback& cb);
    void setMessageCallback(const  Messagecallback& cb);
     
    private:
    void newConnection(int sockfd,const InetAddress& peeraddr);
    void removeConnection(const TcpConnectionptr& conn);
    void removeConnectionInLoop(const TcpConnectionptr& conn);

    EventLoop* loop_;
    bool started_;
      int connid_;
     std::string name_;      
    std::unique_ptr<Acceptor> acceptor_;
    std::unique_ptr<EventLoopThreadPool> threadpool_;
    std::unordered_map<std::string,TcpConnectionptr> connections_;
    InetAddress listenAddr_;
  
    Connectioncallback connectionCallback_;
    Messagecallback messageCallback_;

};
