#pragma once
#include "../common/buffer.h"
#include "../common/protocol/message.h"
#include "../minimuduo/net/channel.h"
#include "../minimuduo/net/socket.h"
#include <functional>
#include <memory>
#include <string>
class EventLoop;
class ClientConnection:public std::enable_shared_from_this<ClientConnection>
{
    public:

    using MessageCallback=std::function<void(const Message&)>;
    ClientConnection(EventLoop* loop,int sockfd);

    ~ClientConnection();
    void send(const Message& message);
    void send(const Message& message, const void*body, size_t len);
    void close();
    void setMessageCallback(MessageCallback cb);
    void connectEstablished();
    void connectDestroyed();
    bool connected() const
    {
    return connected_;
    }
    private:
    void handleRead();
     void handleWrite();
     void handleClose();
     void handleError();
    void sendInLoop(const std::string& data);
     
    private:
    EventLoop* loop_;
     Socket socket_;
    Channel channel_;
    std::unique_ptr<Buffer> inputBuffer_;
    std::unique_ptr<Buffer> outputBuffer_;
    MessageCallback messageCallback_;
    bool connected_{false};

};