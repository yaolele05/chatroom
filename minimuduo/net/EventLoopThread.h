
#pragma once
#include <functional>
#include "noncopyable.h"
#include <mutex>
#include <thread>
#include <condition_variable>
class EventLoop;

class EventLoopThread : noncopyable
{
public:

    using ThreadInitCallback= std::function<void(EventLoop*)>;

    explicit EventLoopThread( const ThreadInitCallback& cb={});

    ~EventLoopThread();

    EventLoop* startLoop();

private:

    void threadFunc();

private:

    EventLoop* loop_;

    bool exiting_;

    bool started_;          

    std::thread thread_;

    std::mutex mutex_;

    std::condition_variable cond_;

    ThreadInitCallback callback_;   
};