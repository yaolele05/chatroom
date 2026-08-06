#pragma once
#include <string>
#include <cstdint>
 #include <string_view>
 #include "../common/buffer.h"
#include <memory>
 class EventLoop;
 class ClientConnection;
class TcpClient
{

public:
    enum class State
    {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

    TcpClient(EventLoop* loop);
    ~TcpClient();

    TcpClient(const TcpClient&) = delete;
    TcpClient& operator=(const TcpClient&) = delete;
    TcpClient(TcpClient&&) = delete;
    TcpClient& operator=(TcpClient&&) = delete;
    bool connect(const std::string& ip, uint16_t port);
    void disconnect();
    std::shared_ptr<ClientConnection> connection() const;
    bool connected() const;
   
private:

     EventLoop* loop_;
     std::shared_ptr<ClientConnection>connection_;
     State state_;
     std::string serverIp_;
      uint16_t serverPort_;
};
