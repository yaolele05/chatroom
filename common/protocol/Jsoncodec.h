#pragma once
#include <string>
#include "message.h"
class JsonCodec
{
    public:
    static std::string encode(const Message& message);
    static Message decode(const std::string& json);

    private:
    static std::string TypeToString(Messagetype type);
    static Messagetype StringToType(const std::string& type);

    static nlohmann::json parseJson(const std::string& data);
    static void dataRequireFields(const nlohmann::json& j);
    static void dataFieldTypes(const nlohmann::json& j);
    static Messagetype parseMessageType(const nlohmann::json& j);
    static void dataMessage(nlohmann::json& j,Messagetype type);
    static Message buildMessage(nlohmann::json& j,Messagetype type);
    
};