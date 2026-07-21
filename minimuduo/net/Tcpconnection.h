#pragma once

#include <functional>
#include <memory>
#include <string>
#include "socket.h"
#include "inetaddress.h"
#include "channel.h"
class EventLoop;

class Buffer;

class TcpConnection:
public std::enable_shared_from_this<TcpConnection>
{
    public:
    using TcpConnectionPtr=std::shared_ptr<TcpConnection>;
    enum State
    {
        kConnecting,
        kConnected,
       
        kDisconnecting,
        kDisconnected
    };
    using TcpConnectionCallback=std::function<void(const TcpConnectionPtr&)>;
    using MessageCallback=std::function<void(const TcpConnectionPtr&,Buffer* buffer)>;
    using CloseCallback=std::function<void(const TcpConnectionPtr&)>;
    using WritecomCallback=std::function<void(const TcpConnectionPtr&)>;
    TcpConnection(EventLoop* loop,int sockfd,const InetAddress& localaddr,const InetAddress& peeraddr);
    ~TcpConnection();
    void send(const std::string& message);
    void sendInLoop(const std::string&msg);
    void shutdown();
    void connEstablished();
    void connDestroyed();
    void setTcpConnectionCallback(const TcpConnectionCallback& cb);
    void setMessageCallback(const MessageCallback& cb);
    void setCloseCallback(const CloseCallback& cb);



    private:
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    EventLoop* loop_;
    Socket socket_;
    Channel channel_;
   InetAddress peerAddr_;
   InetAddress localAddr_;
    State state_;
    std::unique_ptr<Buffer>inputBuffer_;
    std::unique_ptr<Buffer>outputBuffer_;

    TcpConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    CloseCallback closeCallback_;
    WritecomCallback  writecomCallback_;


};