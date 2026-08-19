#include "inetaddress.h"
#include <arpa/inet.h>
#include <string.h>
#include <stdexcept>////
InetAddress::InetAddress()
{
    memset(&addr_,0,sizeof(addr_));
    addr_.sin_family=AF_INET;
}
InetAddress::InetAddress(uint16_t port, const std::string& ip)
{
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &addr_.sin_addr) <= 0)
    {
        throw std::invalid_argument("invalid IPv4 address: " + ip);
    }
}
InetAddress::InetAddress(const sockaddr_in&addr):addr_(addr)
{

}
std::string InetAddress::ip() const
{
    char buf[64];//存放点分十进制的ip地址
    inet_ntop(AF_INET,&addr_.sin_addr,buf,sizeof(buf));
    return buf;
}
uint16_t InetAddress::port() const
{
    return ntohs(addr_.sin_port);
}
socklen_t InetAddress::length() const
{
    return sizeof(addr_);
}
void InetAddress::setSockAddr(const sockaddr_in& addr)
{
        addr_=addr;
}
sockaddr_in* InetAddress::getSockAddr()
{
    return &addr_;
}