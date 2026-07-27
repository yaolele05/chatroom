#include "businessdispatcher.h"

BusinessDispatcher& BusinessDispatcher::instance()
{
    static BusinessDispatcher instance;
    return instance;
}
void BusinessDispatcher::registerHandler(const std::string& type,Handler handler)
{
    handlers_[type]=handler;
}
bool BusinessDispatcher::dispatch(const json& message,Session* session)
{
    auto itType = message.find("type");
     if(itType ==message.end()||itType->is_string())
     {
        return false;
     }
     std::string type=itType->get<std::string>();

     auto it =handlers_.find(type);
     if(it==handlers_.end())
     {
        return false;
     }
     it->second(message,session);
     return true;

}