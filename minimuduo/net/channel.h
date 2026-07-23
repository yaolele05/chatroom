#pragma once

#include <functional>
#include <cstdint>
#include <memory>
class EventLoop;

class Channel
{
public:
 
 enum ChannelState
 {
    kNew=-1,
    kAdded=1,
    kDeleted=2
 };
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop,int fd);
    ~Channel() = default;

    void handleEvent();

    
    void ReadCallback(EventCallback cb);
    void WriteCallback(EventCallback cb);
    void CloseCallback(EventCallback cb);
    void ErrorCallback(EventCallback cb);
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
    bool isNoneEvent() const
    {
        return events_ == 0;
    }
    void tie(const std::shared_ptr<void>& obj);
    void setInEpoll(bool flag)
    {
        inEpoll_= flag;
    }

    int index()const{
        return index_;
    }
    void setIndex(int idx)
    {
        index_=idx;
    }
    // 用户接口
    void enableReading();
    void enableWriting();

    void disableReading();
    void disableWriting();
    void disableAll();
    void remove();

    bool isNoneEvent()
    {
        return events_==0;
    }
    bool isWriting() const;
private:
    EventLoop* loop_;

    int fd_;

    uint32_t events_;
    uint32_t revents_;

    int index_;
    std::weak_ptr<void>tie_;
    bool tied_{false};
    bool inEpoll_;
     
    void handleEventWithGuard();
    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};
