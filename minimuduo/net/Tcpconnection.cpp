#include "Tcpconnection.h"
#include "eventloop.h"
#include "socket.h"
#include "channel.h"
#include "inetaddress.h"
#include "../../common/buffer.h"
#include "callback.h"
#include <unistd.h>
#include <errno.h>
#include <iostream>
#include <cassert>
#include <sys/epoll.h>
#include <cstring>
#include <thread>



TcpConnection::TcpConnection(EventLoop* loop,const std::string& name,int sockfd,const InetAddress& localaddr,const InetAddress& peeraddr):loop_(loop),name_(name),socket_(sockfd),channel_(loop,sockfd),localAddr_(localaddr),peerAddr_(peeraddr),inputBuffer_(new Buffer()),outputBuffer_(new Buffer()),state_(kConnecting)
{
    channel_.ReadCallback(std::bind(&TcpConnection::handleRead,this));
    channel_.WriteCallback(std::bind(&TcpConnection::handleWrite,this));
    channel_.CloseCallback(std::bind(&TcpConnection::handleClose,this));
     channel_.ErrorCallback(std::bind(&TcpConnection::handleError,this));
}
TcpConnection::~TcpConnection()
{

}
void TcpConnection::connEstablished()
{
  state_=kConnected;
  
  channel_.tie(shared_from_this());

  channel_.enableReading();

  if(connectionCallback_)
  {
    connectionCallback_(shared_from_this());
  }
}
void TcpConnection::connDestroyed()
{
   
    if(connectionCallback_)
        {
            connectionCallback_(shared_from_this());
        }

   
   channel_.disableAll();
   channel_.remove();
}
void TcpConnection::setTcpConnectionCallback(const Connectioncallback& cb)
{
    connectionCallback_=cb;
}
void TcpConnection::setMessageCallback(const Messagecallback& cb)
{
    messageCallback_=cb;
}
void TcpConnection::setCloseCallback(const Closecallback& cb)
{
    closeCallback_=cb;
}
void TcpConnection::setWritecomCallback(const Writecomcallback& cb)
{
    writecomCallback_=cb;
}

void TcpConnection::handleRead()
{
    int savedErrno=0;
    ssize_t n=inputBuffer_->readFd(channel_.fd(),&savedErrno);
    if(n>0)
    {
        if(messageCallback_)
        {
            std::cout<<"handleRead thread id:"<<std::this_thread::get_id()<<std::endl;
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
    std::cout<<"handlewrite"<<std::endl;
    if(channel_.revents() & EPOLLOUT)
    {
        int savedErrno=0;

        ssize_t n=outputBuffer_->writeFd(channel_.fd(),&savedErrno);
        if(n>0)
        {
            if(outputBuffer_->readableBytes()==0)
            {
                channel_.disableWriting();

                if(writecomCallback_)
                {
                    writecomCallback_(shared_from_this());
                }
                if(state_==kDisconnecting)
                {
                    shutdown();
                }
            }

             std::cout<<"handleWrite readable="<<outputBuffer_->readableBytes()<<std::endl;
        }
        else
        {
            if(savedErrno!=EWOULDBLOCK)
            {
                handleError();
            }
        }
    }
   
}

void TcpConnection::handleClose()
{
   std::cout << "handleClose thread:" << std::this_thread::get_id()<< std::endl;
    if(state_ == kDisconnected)
    {
        return;
    }
    state_ = kDisconnected;
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
    std::cout<<"Tcpconnection error"<<strerror(err)<<std::endl;
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
    std::cout<<"state="<<state_<<std::endl;
    std::cout<<"channel events="<<channel_.events()<<std::endl;
    std::cout<<"buffer before="<<outputBuffer_->readableBytes()<<std::endl;

   if(state_==kDisconnected)
   {
    return;
   }

   std::cout<<"sendInLoop size="<<msg.size()<<std::endl;

   ssize_t nwrote =0;
   if(!(channel_.events()& EPOLLOUT) && outputBuffer_->readableBytes()==0)
   {
    std::cout<<"try write"<<std::endl;
    nwrote=socket_.write(msg.data(),msg.size());

    std::cout<<"write result="<<nwrote<<" errno="<<errno<<std::endl;

    if(nwrote<0)
    {
        if(errno!= EWOULDBLOCK&& errno!=EAGAIN)
        {
        perror("write");
        }
        nwrote=0;
    }
   }
  
   if(static_cast<size_t>(nwrote)<msg.size())
   {
    outputBuffer_->append(msg.data()+nwrote,msg.size()-nwrote);
    channel_.enableWriting();

    std::cout<<"buffer remain="<<msg.size()-nwrote<<std::endl;

   }
   
}
void TcpConnection::shutdown()
{
    auto self(shared_from_this());
    loop_->runInLoop([self]()
    {
        if(self->state_ == kConnected)
        {
            self->state_ = kDisconnecting;
            if(!self->channel_.isWriting())
            {
                self->socket_.shutdownWrite();
            }
        }
    });
}

void TcpConnection::forceClose()
{
    std::cout << "forceClose thread:"
          << std::this_thread::get_id()
          << std::endl;
    auto self(shared_from_this());
    loop_->runInLoop([self]()
    {
        if(self->state_ == kConnected || self->state_ == kDisconnecting)
        {
            self->handleClose();
        }
    });
}
bool TcpConnection::connected() const
{
        return state_== kConnected;
}
bool TcpConnection::disconnected() const
{
       return state_==kDisconnected;
}
EventLoop* TcpConnection::getLoop() const
{
    return loop_;
}

const std::string& TcpConnection::name() const
{
    return name_;
}

const InetAddress& TcpConnection::localaddress() const
{
    return localAddr_;
}

const InetAddress& TcpConnection::peeraddress() const
{
    return peerAddr_;
}