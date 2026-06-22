#pragma once
#include <functional>
class EventLoop;
class Channel
{
    public:
    Channel(EventLoop*loop,int fd_);

    int fd() const{return fd;};
    
    void setreadcallback(std::function<void()>cb);
    void setwritecallback(std::function<void()>cb);
    void setclosecallback(std::function<void()>cb);

    void handleevent(int events);
    void enableread();
    private:
    EventLoop*loop;
    int fd;
    std::function<void()>readcb;
    std::function<void()>writecb;
    std::function<void()>closecb;
}