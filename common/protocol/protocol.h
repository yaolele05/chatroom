#pragma once
#include<string>
#include <nlohmann/json.hpp>
#include "message.h"
using json=nlohmann::json;
class Protocol
{
    public:
    static Message makeError(int code,const std::string& msg);
    static Message makeSuccess();
};