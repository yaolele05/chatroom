#include "tcpclient.h"
#include<cstdint>
#include<sys/socket.h>
#include <iostream>
#include "../minimuduo/net/channel.h"
#include <arpa/inet.h>
#include<unistd.h>
#include"clientconnection.h"
TcpClient::TcpClient(EventLoop* loop):loop_(loop),state_(State::kDisconnected)
{
   
}
TcpClient::~TcpClient()
{
    disconnect();
}
bool TcpClient::connected() const
{
  return state_==State::kConnected;
}
std::shared_ptr<ClientConnection> TcpClient::connection() const
{
    return connection_;
}
bool TcpClient::connect(const std::string& ip, uint16_t port)
{
    if(state_ != State::kDisconnected)
    {
        return false;
    }
    int sockfd= ::socket(AF_INET,SOCK_STREAM,0);

    if(sockfd<0)
    {
        perror("socket");
        return false;
        
    }

    sockaddr_in serverAddr{};
        serverAddr.sin_family=AF_INET;
        serverAddr.sin_port=htons(port);
        if(::inet_pton(AF_INET,ip.c_str(),&serverAddr.sin_addr)<=0)
        {
            ::close(sockfd);
            return false;
        }
        state_=State::kConnecting;
        if(::connect(sockfd,reinterpret_cast<sockaddr*>(&serverAddr),sizeof(serverAddr))<0) ///有阻塞
        {
            perror("connect");
            ::close(sockfd);
            state_=State::kDisconnected;
            return false;
        }
        connection_=std::make_shared<ClientConnection>(loop_,sockfd);
         connection_->connectEstablished();
         state_=State::kConnected;

         return true;

}

void TcpClient::disconnect()
{
    if(state_ == State::kDisconnected)
    {
        return;
    }
    state_ = State::kDisconnecting;
    if(connection_)
    {
        connection_->close();
        connection_->connectDestroyed();
        connection_.reset();
    }
    state_ = State::kDisconnected;
}
