#pragma once
#include <string>
#include "message.h"
class JsonCodec
{
    public:
    static std::string encode(const Message& message);
    static Message decode(const std::string& json);

    private:
    static std::string TypeToString(MessageType type);
    static MessageType StringToType(const std::string& type);

    static nlohmann::json parseJson(const std::string& data);
    static void dataRequireFields(const nlohmann::json& j);
    static void dataFieldTypes(const nlohmann::json& j);
    static MessageType parseMessageType(const nlohmann::json& j);
    static void dataMessage(nlohmann::json& j,MessageType type);
    static Message buildMessage(nlohmann::json& j,MessageType type);
    
};