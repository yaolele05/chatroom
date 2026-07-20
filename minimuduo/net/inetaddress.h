#pragma once
#include <netinet/in.h>
#include <string>
class InetAddress
{
    public:
    InetAddress();
    InetAddress(uint16_t port,const std::string& ip="0.0.0.0");
    InetAddress(const sockaddr_in& addr);

    const sockaddr_in* getSockAddr() const
    {
        return &addr_;
    }
    std::string ip() const;
    uint16_t port() const;
    socklen_t length() const;
  
    void setSockAddr(const sockaddr_in& addr);
   sockaddr_in* getSockAddr();
 
    private:
    sockaddr_in addr_;
};