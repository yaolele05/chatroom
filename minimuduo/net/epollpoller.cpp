#include "epollpoller.h"
#include "channel.h"
#include <unistd.h> 

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
EpollPoller::EpollPoller():epfd_(::epoll_create1(EPOLL_CLOEXEC)),events_(16)
{
    if(epfd_<0)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
}
EpollPoller::~EpollPoller()
{
    
        ::close(epfd_);
    
}
void EpollPoller::poll(int timeoutMs, ChannelList& activeChannels)
{
    int numEvents=::epoll_wait(epfd_,events_.data(),static_cast<int>(events_.size()),timeoutMs);
    if(numEvents<0)
    {
        if(errno==EINTR)
        {
            return;
        }
        perror("epoll_wait");
       return;
    }
    fillActiveChannels(numEvents,activeChannels);

    if(numEvents==static_cast<int>(events_.size()))
    {
        events_.resize(events_.size()*2);
    }
}
void EpollPoller::fillActiveChannels(int numEvents,ChannelList& activeChannels)
{
    for(int i=0;i<numEvents;i++)
    {
        Channel*channel=static_cast<Channel*>(events_[i].data.ptr);
        channel->setRevents(events_[i].events);
        activeChannels.push_back(channel);
    }
}
void EpollPoller::updateChannel(Channel* channel)
{
    struct epoll_event ev{};
    ev.events=channel->events();
    ev.data.ptr=channel;
    int fd=channel->fd();
    if(channel->index()==-1)
    {
        if(::epoll_ctl(epfd_,EPOLL_CTL_ADD,fd,&ev)<0)
        {
            perror("epoll_ctl add");
           return;
        }
        channel->setInEpoll(true);
        channel->setIndex(1);
    }
    else
    {
        if(channel->isNoneEvent())
        {
            if(::epoll_ctl(epfd_,EPOLL_CTL_DEL,fd,nullptr)<0)
            {
                perror("epoll_ctl del");
                return;
            }
            channel->setInEpoll(false);
            channel->setIndex(-1);
        }
        else
        {
            if(::epoll_ctl(epfd_,EPOLL_CTL_MOD,fd,&ev))
            {
                perror("epoll_ctl mod");
                return;
            }
        }
    }
}
void EpollPoller::removeChannel(Channel* channel)
{
    int fd=channel->fd();
    if(::epoll_ctl(epfd_,EPOLL_CTL_DEL,fd,nullptr)<0)
    {
        perror("epoll_ctl del");
        return;
    }
    channel->setInEpoll(false);
    channel->setIndex(-1);
}