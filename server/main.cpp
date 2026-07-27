#include "chatserver.h"
#include <iostream>

int main()
{
    EventLoop loop;
    InetAddress addr(8888);
    Chatserver server(&loop, addr);
    server.start();
    loop.loop();
    return 0;
}