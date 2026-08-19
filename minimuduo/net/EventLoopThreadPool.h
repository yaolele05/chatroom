#pragma once
#include <memory>
#include <vector>

#include "noncopyable.h"
#include "EventLoopThread.h"
class EventLoop;

class EventLoopThreadPool:noncopyable
{
    public:
    explicit EventLoopThreadPool(EventLoop* mainLoop);
    ~EventLoopThreadPool()=default;

    void setThreadNum(int numThreads);
    void start(const EventLoopThread::ThreadInitCallback& cb={});
    EventLoop* getNextLoop();
    const std::vector<EventLoop*>& getAllLoops() const;
    bool started () const { return started_;}

    private:
    EventLoop* mainLoop_;
    
    bool started_;
     
    int numThreads_;
    int next_;

    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;

};