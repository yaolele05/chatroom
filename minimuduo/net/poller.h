#pragma once 
#include <vector>
class Channel;
class Poller
{
    public:
    virtual ~Poller()=default;
    virtual void poll(int timeoutms,std::vector<Channel*>activeChannels)=0;
    virtual void upateChannel(Channel* channel)=0;
    virtual void removeChannel(Channel* channel)=0;
};