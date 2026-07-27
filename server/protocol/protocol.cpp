#include "protocol.h"
#include <nlohmann/json.hpp>
#include<string>
std::string Protocol::makeError(int code,const std::string& msg)
{
   json j;
   j["code"]=code;
   j["msg"]=msg;
   return j.dump();
}