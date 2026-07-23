#include "TcpServer.h"
#include "eventloop.h"
#include "acceptor.h"
#include"EventLoopThreadPool.h"
#include "inetaddress.h"
#include "Tcpconnection.h"
#include <functional>
#include <iostream>
#include <cassert>
TcpServer::TcpServer(EventLoop* loop,const InetAddress& listenAddress,const std::string& name):
loop_(loop),started_(false),connid_(0),name_(name),acceptor_(new Acceptor(loop,listenAddress)),threadpool_(new EventLoopThreadPool (loop)),listenAddr_(listenAddress)
{

    acceptor_->setNewconnectCallback(std::bind(&TcpServer::newConnection,this,std::placeholders::_1,std::placeholders::_2));
}
void TcpServer::start()
{
    if(started_)
    {
        return;
    }
    started_=true;
    threadpool_->start();
    acceptor_->listen();

}
void TcpServer::setThreadNum(int numthreads)
{
   threadpool_->setThreadNum(numthreads);
}
void TcpServer::setConnectionCallback(const Connectioncallback& cb)
{
  connectionCallback_=cb;
}
void TcpServer::setMessageCallback(const Messagecallback& cb)
{
    messageCallback_=cb;
}
void TcpServer::newConnection(int sockfd,const InetAddress& peeraddress)
{
    std::cout
<<"accept thread:"
<<std::this_thread::get_id()
<<std::endl;
    EventLoop* ioLoop=threadpool_->getNextLoop();
    std::cout
<<"ioLoop thread:"
<<ioLoop->threadId()
<<std::endl;
   std::string conname ="conn#" + std::to_string(connid_++);
    TcpConnectionptr conn(new TcpConnection(ioLoop,conname,sockfd,listenAddr_,peeraddress));
   
    connections_[conname]=conn;
    conn->setTcpConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection,this,std::placeholders::_1));
    ioLoop->runInLoop(std::bind(&TcpConnection::connEstablished,conn));
    
}
void TcpServer::removeConnection(const TcpConnectionptr& conn)
{
     std::cout
    <<"removeConnection thread:"
    <<std::this_thread::get_id()
    <<std::endl;

    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop,this,conn));
}
void TcpServer::removeConnectionInLoop(const TcpConnectionptr& conn)
{
        std::cout
    <<"removeConnectionInLoop thread:"
    <<std::this_thread::get_id()
    <<std::endl;
    connections_.erase(conn->name());
    EventLoop*ioLoop=conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connDestroyed,conn));
}
TcpServer::~TcpServer()
{

}