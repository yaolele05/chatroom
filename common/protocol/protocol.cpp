#include "protocol.h"
#include <nlohmann/json.hpp>
#include<string>
Message Protocol::makeError(int code,const std::string& msg)
{
   Message message;
   message.setType(Messagetype::Error);
   auto& payload=message.payload();

   payload["code"]=code;
   payload["msg"]=msg;

   return message;

}
Message Protocol::makeSuccess()
{
    Message message;
    message.setType( Messagetype::MessageAck);
    message.setTimestamp(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    message.payload()["code"] = 0;
    message.payload()["message"] = "success";
    return message;
}