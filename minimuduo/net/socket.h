#pragma once
class InetAddress;

class Socket
{
    public:
    static int createNonblockingSocket();
    explicit Socket(int fd);////
    ~Socket();
    int fd()const;////
    void bindAddress(const InetAddress& addr);
    void listen();
    int accept(InetAddress* peeraddr);
    void setReuseAddr(bool on);
    void setReusePort(bool on);
    void shutdownWrite();
   // void setKeepAlive(bool on);////
    private:
    int fd_;
    Socket(const Socket&)=delete;
    Socket& operator=(const Socket&)=delete;
};