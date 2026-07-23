#pragma once
#include <functional>
#include "socket.h"
#include "inetaddress.h"
#include "eventloop.h"
#include "channel.h"

class Acceptor
{
    public:
    using NewconnectCallback=std::function<void(int sockfd,const InetAddress& peeraddr)>;
    Acceptor(EventLoop* loop,const InetAddress& listenaddr);
    ~Acceptor();
    void listen();
    void setNewconnectCallback(const NewconnectCallback& cb);

    private:
    void handleReading();

    EventLoop* loop_;
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewconnectCallback newconnectCallback_;
    bool listening_;

    
};