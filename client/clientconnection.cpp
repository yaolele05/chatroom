#include "clientconnection.h"
#include "../common/protocol/Jsoncodec.h"
#include "../common/protocol/packetcodec.h"
#include "../minimuduo/net/eventloop.h"
#include<arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <nlohmann/json.hpp>
#include <cstdlib>
ClientConnection::ClientConnection(EventLoop* loop,int sockfd):loop_(loop),socket_(sockfd),channel_(loop,sockfd),inputBuffer_(new Buffer()),outputBuffer_(new Buffer())
{
    channel_.ReadCallback(std::bind(&ClientConnection::handleRead,this));
    channel_.WriteCallback(std::bind(&ClientConnection::handleWrite,this));
    channel_.CloseCallback(std::bind(&ClientConnection::handleClose,this));
    channel_.ErrorCallback(std::bind(&ClientConnection::handleError,this));
    
}
ClientConnection::~ClientConnection()
{
}
void ClientConnection::setMessageCallback(MessageCallback cb)
{
    messageCallback_ = std::move(cb);
}
void ClientConnection::send(const Message& msg)
{
     send(msg,nullptr,0);
}
void ClientConnection::send(const Message& msg,const void* body,size_t len)
{
   std::string json=JsonCodec::encode(msg);
   std::string packet=PacketCodec::encode(json,body,len);
   if(loop_->isInLoopThread())
   {
    sendInLoop(packet);
   }
   else
   {
    auto self=shared_from_this();
    loop_->queueInLoop([self,packet]()
   {
    self->sendInLoop(packet);
   });
   }
}
void ClientConnection::sendInLoop(const std::string& data)
{
    ssize_t nwrote=0;
    if(!(channel_.events()& EPOLLOUT)&& outputBuffer_->readableBytes()==0)
    {
        nwrote=socket_.write(data.data(),data.size());
        if(nwrote<0)
        {
            if(errno!=EWOULDBLOCK && errno != EAGAIN)
            {
                perror("write");
            }
            nwrote=0;
        }
    }
    if(static_cast<size_t>(nwrote)<data.size())
    {
        outputBuffer_->append(data.data()+nwrote,data.size()-nwrote);
        channel_.enableWriting();
    }
}
void ClientConnection::handleWrite()
{
    if(channel_.revents() & EPOLLOUT)
   {
        int savedErrno = 0;
        ssize_t n =outputBuffer_->writeFd( channel_.fd(),&savedErrno);
        if(n > 0)
        {
        if(outputBuffer_->readableBytes()==0)
        {
          channel_.disableWriting();
        }
        }
        else
        {
         if(savedErrno != EWOULDBLOCK)
          {
            handleError();
          }
        }
  }
}
 void ClientConnection::setCloseCallback(CloseCallback cb)
{
      closeCallback_ = std::move(cb);
}

void ClientConnection::handleClose()
{
    connected_=false;
    channel_.disableAll();
    channel_.remove();
     if (closeCallback_) closeCallback_(); 
}
void ClientConnection::handleError()
{
    int err=0;
    socklen_t len=sizeof(err);
    getsockopt(channel_.fd(),SOL_SOCKET,SO_ERROR,&err,&len);
    std::cout<<"Tcpconnection error"<<strerror(err)<<std::endl;
}
void ClientConnection::close()
{
    handleClose();
}

void ClientConnection::connectEstablished()
{
    connected_=true;
    channel_.tie(shared_from_this());
    channel_.enableReading();
    std::cout<<"events="<<channel_.events()<<std::endl;
}

void ClientConnection::connectDestroyed()
{
    channel_.disableAll();
    channel_.remove();
}
void ClientConnection::handleRead()
{
   int savedErrno=0;
    ssize_t n=inputBuffer_->readFd(channel_.fd(),&savedErrno);
    if(n>0)
    {
    
        /* auto data=inputBuffer_->peek();
        uint32_t len;
       memcpy(&len,data,4);*/
        while(true)
        {
        std::string json;
        std::string body;
        PacketCodec::DecodeResult result=PacketCodec::decode(*inputBuffer_,json,&body);
        switch(result)
        {
            case PacketCodec::DecodeResult::Ok:
            {
                try
                {

                   Message message =JsonCodec::decode(json);
                    message.setBinary(std::move(body));
                    messageCallback_(message);
                   
                }
                catch(const std::exception& e)
                {
                    std::cerr<<"json decode error"<<e.what()<<std::endl;
                    handleClose();
                    return;
                }
                continue;
            }
            case PacketCodec::DecodeResult::Needmoredata:
            {
                //半包
                return;
            }
            case PacketCodec::DecodeResult::ProtocolError:
            {
                std::cerr<<"protocol error"<<std::endl;
                handleClose();
                return;
            }
                
         }
       }
    }
    else if(n==0)
    {
        handleClose();
        return ;
    }
    else
    {
        if(savedErrno !=EAGAIN && savedErrno !=EWOULDBLOCK && savedErrno != EINTR)
        {
            handleError();
        }
    }
}
