#include "Tcpconnection.h"
#include "eventloop.h"
#include "socket.h"
#include "channel.h"
#include "inetaddress.h"
#include "buffer.h"

#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <cassert>

TcpConnection::TcpConnection(EventLoop* loop,int sockfd,const InetAddress& localaddr,const InetAddress& peeraddr):loop_(loop),socket_(sockfd),channel_(loop,sockfd),localAddr_(localaddr),peerAddr_(peeraddr),inputBuffer_(new Buffer()),outputBuffer_(new Buffer()),state_(kConnecting)
{
    channel_.setReadCallback(std::bind(&TcpConnection::handleRead,this));
    channel_.setWriteCallback(std::bind(&TcpConnection::handleWrite,this));
    channel_.setCloseCallback(std::bind(&TcpConnection::handleClose,this));
}
TcpConnection::~TcpConnection()
{

}
void TcpConnection::connEstablished()
{
  state_=kConnected;
  
  channel_.enableReading();
  if(connectionCallback_)
  {
     connectionCallback_(shared_from_this());
  }

}
void TcpConnection::connDestroyed()
{
   if(state_==kConnected)
   {
    state_=kDisconnected;

   }
   channel_.disableAll();
}
void TcpConnection::handleRead()
{
    int savedErrno=0;
    ssize_t n=inputBuffer_->readFd(channel_.fd(),&savedErrno);
    if(n>0)
    {
        if(messageCallback_)
        {
            messageCallback_(shared_from_this(),inputBuffer_.get());
        }
    }
    else if(n==0)
    {
        handleClose();
    }
    else
    {
        handleError();
    }
}
void TcpConnection::handleWrite()
{
    ssize_t n=::write(channel_.fd(),outputBuffer_->peek(),outputBuffer_->readableBytes());
     if(n>0)
     {
        outputBuffer_->retrieve(n);
        if(outputBuffer_->readableBytes()==0)
        {
            channel_.disableWriting();
        }
     }
}
void TcpConnection::handleClose()
{
    state_=kDisconnected;
    channel_.disableAll();
    if(closeCallback_)
    {
        closeCallback_(shared_from_this());
    }


}
void TcpConnection::handleError()
{
    int err=0;
    socklen_t len=sizeof(err);
    getsockopt(channel_.fd(),SOL_SOCKET,SO_ERROR,&err,&len);

}
void TcpConnection::send(const std::string& msg)
{
    if(loop_->isInLoopThread())
    {
        sendInLoop(msg);
    }
    else
    {
        auto self(shared_from_this());

        loop_->queueInLoop([self,msg](){
            self->sendInLoop(msg);
        });
    }
}
void TcpConnection::sendInLoop(const std::string&msg)
{
   if(state_==kDisconnected)
   {
    return;
   }
   ssize_t nwrote =0;
   if(!(channel_.events()& EPOLLOUT) && outputBuffer_->readableBytes()=0)
   {
    nwrote=::write(channel_.fd(),msg.data(),msg.size());

    if(nwrote<0)
    {
        if(errno!= EWOULDBLOCK)
        {
        perror("write");
        }
        nwrote=0;
    }
   }
   if(static_cast<size_t>(nwrote))
   {
    outputBuffer_->append(msg.data()+nwrote,msg.size()-nwrote);
    channel_.enableWriting();
   }

}
void TcpConnection::shutdown()
{
    if(state_==kConnected)
    {
        state_=kDisconnecting;
        socket_.shutdownWrite();
    }
}
 