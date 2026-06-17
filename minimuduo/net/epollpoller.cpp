#include "epollpoller.h"
#include "channel.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
EpollPoller::EpollPoller():epfd(::epoll_create1(EPOLL_CLOEXEC)),Events(1024){}
EpollPoller::~EpollPOller()
{
    ::close(epfd);
}
void EpollPoller::updateChannel(Channel*ch)
{
    epoll_event ev;
    std::memset(&ev,0,sizeof(ev));
     ev.data.ptr=ch;
     ev.events=ch->events();
     int fd=ch->fd();
     if(!ch->inEPOLL())
     {
        ::epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&ev);
        ch->setINEPOLL(true);
     }
     else
     {
        ::epoll_ctl(epfd,EPOLL_CTL_MOD,fd,&ev);
     }
}
void EpollPOller::removeChannel(Channel* ch)
{
    int fd=ch->fd();
    ::epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&ev);
    ch->setINPOLL(false);
}
void EpollPoller::poll(int timeoutms,std::vector<Channel*>& activeChannels)
{
    int n=::epoll_wait(epfd,Events.data(),static_cast<int>(Events.size()),timeoutms);
    if(n>0)
    {
        fillactiveChannels(n,activeChannels);
    }
}
void EpollPller::fillactiveChannels(int numevents,std::vector<Channel*>&activeChannels)
{
    for(int i=0;i<numevents;i++)
    {
        auto* ch=static_cast<Channel*>(Event[i].data.ptr);
        ch->setRevents(Events[i].events);

        activeChannels.push_back(ch);
    }
}