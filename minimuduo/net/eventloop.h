#pragma once
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <functional>

#include "poller.h"
class Channel;
class Poller;

class EventLoop
{
    public:
    using Functor=std::function<void()>;

    EventLoop();
    ~EventLoop();

    void loop();
    void quit();

    bool callingPendingFunctors_;
    
    void updateChannel(Channel* channel);
    void removeChannel(Channel*channel);
    
    bool isInLoopThread() const;
       
    void runInLoop(Functor cb);
    void queueInLoop(Functor cb);
  

    private:

    bool looping_;
    int wakeupfd_;
    bool quit_;
    std::unique_ptr<Channel>wakeupChannel_;

    std::thread::id threadId_;

    std::unique_ptr<Poller> poller_;
   
    Poller::ChannelList activeChannels_;

    std::mutex mutex_;
    std::vector<Functor> pendingFunctors_;
    void wakeup();
    void handleRead();
    void doPendingFunctors();
};