#pragma once
#include <memory>
#include <vector>
#include <thread>
#include "poller.h"
class Channel;
class Poller;

class EventLoop
{
    public:
    EventLoop();
    ~EventLoop();

    void loop();
    void quit();
    void updateChannel(Channel* channel);
    void removeChannel(Channel*channel);
    
    bool isInLoopThread() const;

    private:
    bool looping_;
    bool quit_;
    std::thread::id threadId_;
    std::unique_ptr<Poller> poller_;
    Poller::ChannelList activeChannels_;
};