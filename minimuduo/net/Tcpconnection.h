#pragma once
#include "callback.h"
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

    enum State
    {
        kConnecting,
        kConnected,
       
        kDisconnecting,
        kDisconnected
    };
 

    TcpConnection(EventLoop* loop,const std::string& name, int sockfd,const InetAddress& localaddr,const InetAddress& peeraddr);
    ~TcpConnection();
    void send(const std::string& message);
    void sendInLoop(const std::string&msg);
    void shutdown();

    void connEstablished();
    void connDestroyed();

    void setTcpConnectionCallback( const Connectioncallback& cb);
    void setMessageCallback(const Messagecallback& cb);
    void setCloseCallback(const  Closecallback& cb);
    void setWritecomCallback(const  Writecomcallback& cb);

    void forceClose();
    bool connected()const;
    bool disconnected() const;

    EventLoop* getLoop() const;
    const std::string& name() const;
    const InetAddress& localaddress()const;
    const InetAddress& peeraddress() const;

    private:
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();

    EventLoop* loop_;
    std::string name_;
    Socket socket_;
    Channel channel_;
    InetAddress peerAddr_;
    InetAddress localAddr_;
    State state_;
    std::unique_ptr<Buffer>inputBuffer_;
    std::unique_ptr<Buffer>outputBuffer_;

    Connectioncallback connectionCallback_;
    Messagecallback messageCallback_;
    Closecallback closeCallback_;
    Writecomcallback  writecomCallback_;

    
};