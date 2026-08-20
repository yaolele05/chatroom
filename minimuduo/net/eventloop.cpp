#include "eventloop.h"
#include "channel.h"
#include "epollpoller.h"
#include <unistd.h>
#include <cassert>////
#include <atomic>
#include <sys/eventfd.h>
#include <cstdio>
#include <thread>
#include <chrono>
static int createEventfd()
{
    int fd=eventfd(0,EFD_NONBLOCK | EFD_CLOEXEC);
    if(fd<0)
    {
        abort();

    }
    return fd;
}
EventLoop::EventLoop():looping_(false),wakeupfd_(createEventfd()),quit_(false),callingPendingFunctors_(false),threadId_(std::this_thread::get_id()),poller_(new EpollPoller()),wakeupChannel_(new Channel(this,wakeupfd_)),lastTimerCheck_(std::chrono::steady_clock::now()){


    wakeupChannel_->ReadCallback(std::bind(&EventLoop::handleRead,this));
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
    auto now = std::chrono::steady_clock::now();
    if(now - lastTimerCheck_ >= std::chrono::seconds(10))
    {
         lastTimerCheck_ = now;
        if(timerCallback_)
        {
       timerCallback_();
         }
        }
    }
    looping_=false;
}
void EventLoop::quit()
{
   quit_=true;
   if(!isInLoopThread())
   {
    wakeup();
   }
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

    callingPendingFunctors_=true;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(auto& functor:functors)
    {
        functor();
    }
}
std::thread::id EventLoop::threadId() const
{
    return threadId_;
} 
void EventLoop::setTimerCallback(Functor cb)
{
    timerCallback_ = std::move(cb);
}