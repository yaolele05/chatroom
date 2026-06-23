#include"channel.h"
#include "eventloop.h"
#include <sys/epoll.h>
Channel::Channel(EventLoop* loop,int fd): loop_(loop),fd_(fd),events_(0),revents_(0),inEpoll_(false){}
void Channel::handleEvent()
{
    if(revents_&(EPOLLERR|EPOLLHUP))
    {
        if(closeCallback_)
        {
            closeCallback_();
        }
        return;
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
