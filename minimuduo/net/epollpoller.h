#pragma once

#include "poller.h"
#include <vector>
#include <sys/epoll.h>

class Channel;

class EpollPoller : public Poller
{
public:
    EpollPoller();
    ~EpollPoller() override;

    EpollPoller(const EpollPoller&) = delete;
    EpollPoller& operator=(const EpollPoller&) = delete;

    void poll(int timeoutMs, std::vector<Channel*>& activeChannels) override;

    void updateChannel(Channel* channel) override;
    void removeChannel(Channel* channel) override;
   
    
private:
    void fillActiveChannels(int numEvents, ChannelList& activeChannels);


    int epfd_;   

    std::vector<epoll_event> events_;
};