#include"channel.h"
#include "eventloop.h"
#include <sys/epoll.h>
Channel::Channel(EventLoop* loop,int fd): loop_(loop),fd_(fd),events_(0),revents_(0),inEpoll_(false){}
void Channel::setReadCallback(EventCallback cb)
{
     readCallback_=std::move(cb);
}
void Channel::setWriteCallback(EventCallback cb)
{
    writeCallback_=std::move(cb);
}
void Channel::setCloseCallback(EventCallback cb)
{
    closeCallback_=std::move(cb);
}
void Channel::handleEvent()
{
    if(revents_ & EPOLLHUP)
    {
        if(closeCallback_)
        {
            closeCallback_();
        }
    }
    if(revents_&EPOLLIN)
    {
        if(readCallback_)
        {
            readCallback_();
        }
    }
    if(revents_&EPOLLOUT)
    {
        if(writeCallback_)
        {
            writeCallback_();
        }
    }

}
void Channel::enableReading()
{
    events_ |= EPOLLIN;
    loop_->updateChannel(this);
}
void Channel::enableWriting()
{
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