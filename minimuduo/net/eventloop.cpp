#include "eventloop.h"
#include "channel.h"
#include "epollpoller.h"

#include<cassert>////
EventLoop::EventLoop():looping_(false),quit_(false),threadId_(std::this_thread::get_id()),poller_(new EpollPoller()){}
void EventLoop::loop()
{
    looping_=true;
    while(!quit_)
    {
    activeChannels_.clear();
    poller_->poll(1000,activeChannels_);
    for(Channel* channel:activeChannels_)
    {
        channel->handleEvent();
    }
    }
    looping_=false;
}
void EventLoop::quit()
{
    quit_=true;
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