#pragma once

#include <functional>
#include <cstdint>

class EventLoop;

class Channel
{
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop,int fd);
    ~Channel() = default;

    void handleEvent();

    
    void setReadCallback(EventCallback cb);
    void setWriteCallback(EventCallback cb);
    void setCloseCallback(EventCallback cb);

    // fd
    int fd() const
    {
        return fd_;
    }

    // epoll events
    uint32_t events() const
    {
        return events_;
    }

    uint32_t revents() const
    {
        return revents_;
    }

    void setRevents(uint32_t ev)
    {
        revents_ = ev;
    }

    // 状态控制
    bool inEpoll() const
    {
        return inEpoll_;
    }

    void setInEpoll(bool flag)
    {
        inEpoll_= flag;
    }

    // 用户接口
    void enableReading();
    void enableWriting();

    void disableWriting();
    void disableAll();

private:
    EventLoop* loop_;

    int fd_;

    uint32_t events_;
    uint32_t revents_;

    bool inEpoll_;

    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
};
