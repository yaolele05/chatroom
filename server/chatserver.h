#pragma once

#include "../minimuduo/net/eventloop.h"
#include "../minimuduo/net/TcpServer.h"
#include "../minimuduo/net/callback.h"
#include "../minimuduo/net/Tcpconnection.h"
#include "../minimuduo/net/inetaddress.h"
#include "../common/buffer.h"
#include "business/businessdispatcher/businessdispatcher.h"

#include <memory>
#include <string>


class Chatserver
{
public:

    Chatserver(EventLoop* loop,const InetAddress& addr);
    void setThreadNum(int num);
    void start();
    
    void checkUsers();

private:

    void onConnection(const TcpConnectionptr& conn);
    void onMessage(const TcpConnectionptr& conn,Buffer* buffer);

    EventLoop* loop_;
    std::unique_ptr<TcpServer> server_;
  

};