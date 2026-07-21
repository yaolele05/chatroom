#include "eventloop.h"
#include "channel.h"
#include "epollpoller.h"
#include <unistd.h>
#include <cassert>////
#include <atomic>
#include <sys/eventfd.h>
#include <cstdio>
static int createEventfd()
{
    int fd=eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if(fd<0)
    {
        abort();

    }
    return fd;
}
EventLoop::EventLoop():looping_(false),wakeupfd_(createEventfd()),quit_(false),callingPendingFunctors_(false),threadId_(std::this_thread::get_id()),poller_(new EpollPoller()),wakeupChannel_(new Channel(this,wakeupfd_)){


    wakeupChannel_->setReadCallback(std::bind(&EventLoop::handleRead,this));
    wakeupChannel_->enableReading();
}
EventLoop::~EventLoop()
{
    close(wakeupfd_);
}
void EventLoop::loop()
{
    looping_=true;
    while(!quit_)
    {
    activeChannels_.clear();
    poller_->poll(10000,activeChannels_);
    for(Channel* channel:activeChannels_)
    {
        channel->handleEvent();
    }

    doPendingFunctors();
    }
    looping_=false;
}
void EventLoop::quit()
{
    
    std::atomic<bool>quit_;
}

void EventLoop::updateChannel(Channel* channel)
{
    poller_->updateChannel(channel);

}
void EventLoop::removeChannel(Channel*channel)
{
    poller_->removeChannel(channel);

}
bool EventLoop::isInLoopThread()const{
    return threadId_==std::this_thread::get_id();
}
void EventLoop::runInLoop(Functor cb)
{
    if(isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
}
void EventLoop::queueInLoop(Functor cb)
{
    {
        std::lock_guard<std::mutex>lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    if(!isInLoopThread()||callingPendingFunctors_)
    {
    wakeup();
    }
}
void EventLoop::wakeup()
{
    uint64_t one=1;

    ssize_t n=write(wakeupfd_,&one,sizeof(one));

    if(n!=sizeof(one))
    {
       if(errno!=EAGAIN)
       {
        perror("eventfd write");
       }
    }
}
void EventLoop::handleRead()
{
    uint64_t one;
    ssize_t n=::read(wakeupfd_,&one,sizeof(one));
    if(n!=sizeof(one))
    {
        perror("eventfd read");
    }
}
void EventLoop::doPendingFunctors()
{
    std::vector<Functor>functors;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(auto& functor:functors)
    {
        functor();
    }
}