#include "businessdispatcher.h"
#include <cerrno>
#include <iostream>
#include <mutex>
BusinessDispatcher& BusinessDispatcher::instance()
{
    static BusinessDispatcher instance;
    return instance;
}
void BusinessDispatcher::registerHandler(const Messagetype type, Handler han)
{
 
    std::lock_guard<std::mutex>lock(mutex_);
    handlers_[type]=std::move(han);


    return;
}
bool BusinessDispatcher::dispatch(const Message& msg ,Session* se)
{
     std::cout<<"dispatch message type="<<static_cast<int>(msg.type())<<std::endl;
   Handler handler;
   {

    std::lock_guard<std::mutex>lock(mutex_);
    auto it=handlers_.find(msg.type());
    if(it==handlers_.end())
    {
         std::cout<<"handler not found type=" <<static_cast<int>(msg.type())<<std::endl;
        return false;
    }
    handler=it->second;
   }
    std::cout<<"execute handler type="<<static_cast<int>(msg.type())<<std::endl;
    handler(msg,se);
    return true;
}