#include "EventLoopThread.h"
#include "eventloop.h"
#include <cassert>
EventLoopThread::EventLoopThread(const ThreadInitCallback& cb):loop_(nullptr),exiting_(false),started_(false),callback_(cb)
{

}

EventLoopThread::~EventLoopThread()
{
    exiting_=true;
   

    {
        std::lock_guard<std::mutex> lock(mutex_);

    if(loop_ != nullptr)
    {
        loop_->quit();
    }
    }


    if(thread_.joinable())
    {
        thread_.join();
    }
}
EventLoop* EventLoopThread::startLoop()
{
    assert(!started_);
    started_=true;
    thread_=std::thread(&EventLoopThread::threadFunc,this);
    std::unique_lock<std::mutex> lock(mutex_);
   cond_.wait(lock,[this]
    {
      return loop_!=nullptr;
     });
    return loop_;

}
void EventLoopThread::threadFunc()
{
    EventLoop loop;
   {
     std::lock_guard<std::mutex> lock(mutex_);
     loop_=&loop;
     cond_.notify_one();
    }
    
   if(callback_)
   {
    callback_(&loop);
   }
    

    loop.loop();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        loop_=nullptr;
    }

}