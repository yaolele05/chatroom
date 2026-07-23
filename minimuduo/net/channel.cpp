#include"channel.h"
#include "eventloop.h"
#include <sys/epoll.h>
#include <iostream>
Channel::Channel(EventLoop* loop,int fd): loop_(loop),fd_(fd),events_(0),revents_(0),index_(kNew),inEpoll_(false){}
void Channel::ReadCallback(EventCallback cb)
{
     readCallback_=std::move(cb);
}
void Channel::WriteCallback(EventCallback cb)
{
    writeCallback_=std::move(cb);
}
void Channel::CloseCallback(EventCallback cb)
{
    closeCallback_=std::move(cb);
}
void Channel::ErrorCallback(EventCallback cb)
{
    errorCallback_=std::move(cb);
}
void Channel::handleEvent()
{
   std::shared_ptr<void> guard;
   if(tied_)
   {
    guard=tie_.lock();
    if(!guard)
    {
        return;
    }
   }
    handleEventWithGuard();
}


void Channel::handleEventWithGuard()
{
    if(revents_& EPOLLHUP)
    {
     if(closeCallback_)
     {
        closeCallback_();
     }
    }
    if(revents_ &EPOLLERR)
    {
        if(errorCallback_)
        {
            errorCallback_();
        }
    }
    if(revents_ &EPOLLIN)
    {
        if(readCallback_)
        {
            readCallback_();
        }

    }
    if(revents_ & EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }
    
}
void Channel::tie(const std::shared_ptr<void>& obj)
{
    tie_=obj;
    tied_=true;
}
void Channel::enableReading()
{
    events_ |= EPOLLIN;
    loop_->updateChannel(this);
}
void Channel::enableWriting()
{
    std::cout<<"enable EPOLLOUT"<<std::endl;
    events_ |= EPOLLOUT;
    loop_->updateChannel(this);
}
void Channel::disableReading()
{
    events_ &= ~EPOLLIN;
    loop_->updateChannel(this);
}
void Channel::disableWriting()
{
    events_ &= ~EPOLLOUT;
    loop_->updateChannel(this);
}
void Channel::disableAll()
{
    events_=0;
    loop_ ->updateChannel(this);
}
void Channel::remove()
{
    loop_->removeChannel(this);
}
bool Channel::isWriting() const
{
    return events_ & EPOLLOUT;
}