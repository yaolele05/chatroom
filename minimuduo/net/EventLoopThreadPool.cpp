#include "EventLoopThreadPool.h"
#include "EventLoopThread.h"
#include "noncopyable.h"
#include <cassert>


EventLoopThreadPool::EventLoopThreadPool(EventLoop* mainLoop):mainLoop_(mainLoop),started_(false),numThreads_(0),next_(0)
{

}



void EventLoopThreadPool::setThreadNum(int numThreads)
{
    assert(!started_);
    assert(numThreads >= 0);

    numThreads_=numThreads;
}



void EventLoopThreadPool::start(const EventLoopThread::ThreadInitCallback& cb)
{
    assert(!started_);

    started_=true;
    for(int i=0;i<numThreads_;++i)
    {
        auto thread=std::make_unique<EventLoopThread>(cb);
        EventLoop*loop=thread->startLoop();
        loops_.push_back(loop);
        threads_.push_back(std::move(thread));
    }
}
EventLoop* EventLoopThreadPool::getNextLoop()
{
    if(loops_.empty())
    {
        return mainLoop_;
    }
    EventLoop* loop=loops_[next_];
    ++next_;
    if(next_ >=static_cast<int>(loops_.size()))
    {
     next_=0;
    }
    return loop;
}
const std::vector<EventLoop*>& EventLoopThreadPool::getAllLoops() const
{
    return loops_;
}