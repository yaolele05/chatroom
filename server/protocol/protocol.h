#pragma once
#include<string>
#include <nlohmann/json.hpp>

using json=nlohmann::json;
class Protocol
{
    public:
    static std::string makeError(int code,const std::string& msg);
    static std::string makeSuccess(const json& data);
};