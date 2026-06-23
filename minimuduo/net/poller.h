#pragma once

#include <vector>

class Channel;

class Poller
{
public:
    Poller() = default;
    virtual ~Poller() = default;

    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

    // 等待事件发生（epoll_wait 封装）
    virtual void poll(int timeoutMs,std::vector<Channel*>& activeChannels) = 0;

    // 更新 Channel（add / mod）
    virtual void updateChannel(Channel* channel) = 0;

    // 删除 Channel
    virtual void removeChannel(Channel* channel) = 0;
};