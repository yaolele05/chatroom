#include "Jsoncodec.h"
#include <stdexcept>

using json = nlohmann::json;
std::string JsonCodec::TypeToString(MessageType type)
{
    switch(type)
    {
     case MessageType::Register:return "register";
     case MessageType::Login:return "login";
     case MessageType::Logout:return "logout";
     case MessageType::RegisterResponse:return "register_response";
     case MessageType::LoginResponse:return "login_response";
     case MessageType::AddFriend:return "addfriend";
     case MessageType::DeleteFriend:return "delete_friend";
     case MessageType::FriendList:return "friendlist";
     case MessageType::FriendOnlineStatus:return "friendonlinestatus";
    
     case MessageType::PrivateChat:return "pravite_chat";

     
     case MessageType::CreateGroup:return "create_group";
     case MessageType::JoinGroup:return "joingroup";
    case MessageType::LeaveGroup: return"leavegroup";
     case MessageType::GroupChat:return "group_chat";
     
     case MessageType::HeartBeat: return "heartbeat";

    case MessageType::FileStart: return "file_start";
    case MessageType::FileChunk: return "file_chunk";
    case MessageType::FileFinish: return "file_finish";
    case MessageType::MessageAck: return "message_ack";
    case MessageType::MessageRecall:return "message_recall";
    case MessageType::MessageRead: return "message_read";
    case MessageType::Error: return "error";

    default: return"unknown";
    }
}
MessageType JsonCodec::StringToType(const std::string& type)
{
     if(type=="register")
        return MessageType::Register;
    if(type=="login")
        return MessageType::Login;

    if(type=="logout")
        return MessageType::Logout;
    if(type=="register_response")
        return MessageType::RegisterResponse;
    if(type=="login_response")
        return MessageType::LoginResponse;
    if(type=="add_friend")
        return MessageType::AddFriend;

    if(type=="delete_friend")
        return MessageType::DeleteFriend;

    if(type=="friend_list")
        return MessageType::FriendList;
    if(type=="friend_online_status")
        return MessageType::FriendOnlineStatus;

    if(type=="private_chat")
        return MessageType::PrivateChat;

    if(type=="create_group")
        return MessageType::CreateGroup;
    if(type=="join_group")
        return MessageType::JoinGroup;

    if(type=="leave_group")
        return MessageType::LeaveGroup;

    if(type=="group_chat")
        return MessageType::GroupChat;
    if(type=="heartbeat")
        return MessageType::HeartBeat;
    if(type=="file_start")
        return MessageType::FileStart;
    if(type=="file_chunk")
        return MessageType::FileChunk;

    if(type=="file_finish")
        return MessageType::FileFinish;
    if(type=="message_ack")
        return MessageType::MessageAck;
    if(type=="message_recall")
        return MessageType::MessageRecall;

    if(type=="message_read")
        return MessageType::MessageRead;
    if(type=="error")
        return MessageType::Error;
    return MessageType::unknown;
}
std::string JsonCodec::encode(const Message& message)
{
    json j;
    j["type"] =TypeToString( message.type());
    j["sequence"] =message.sequence();

    j["senderId"] =message.senderId();

    j["receiverId"] = message.receiverId();

    j["timestamp"] = message.timestamp();
    j["payload"] = message.payload();
    return j.dump();
}

nlohmann::json JsonCodec::parseJson(const std::string& data)
{
    json j;
    try
    {
         j=json::parse(data);
    }
    catch(const json::exception& e)
    {
         throw std::runtime_error("invalid json"+std::string(e.what()));
    }
    return j;
}
void JsonCodec::dataRequireFields(const nlohmann::json& j)
{
    static constexpr const char* fields[]={  "type" "sequence","senderId","receiverId","timestamp", "payload"};
     for(const char*f:fields)
     {
        if(!j.contains(f))
        {
            throw std::runtime_error("false fields"+std::string(f));
        }
     }


}
void JsonCodec::dataFieldTypes(const nlohmann::json& j)
{
    if (!j["type"].is_string())
     {
         throw std::runtime_error("type erro") ;       
    }
    if(!j["sequence"].is_number_unsigned())
    {
        throw std::runtime_error("sequence must be uint64_t");
    }
    if(!j["senderId"].is_number_unsigned())
    {
        throw std::runtime_error("sendId must be uint32_t");
    }
    if(!j["receiverId"].is_number_unsigned())
    {
        throw std::runtime_error("receiverId must be uint32_t");
    }
    if(!j["timestamp"].is_number_unsigned())
    {
        throw std::runtime_error("timestamp must be uint64_t");
    }
    if(!j["payload"].is_object())
    {
        throw std::runtime_error("payload must be object");
    }
}
MessageType JsonCodec::parseMessageType(const nlohmann::json& j)
{
    MessageType type=StringToType(j.at("type").get<std::string>());
    if(type == MessageType::unknown)
    { 
     throw std::runtime_error("unknown Message type");
    }
    return type;
}
void JsonCodec::dataMessage(nlohmann::json& j,MessageType type)
{
    switch(type)
    {
        case MessageType::Login:
        {
            if(!j["paylode"].contains("username"))
            {
                throw std::runtime_error("login no username");
            }
             if(!j["paylode"].contains("password"))
             {
               throw std::runtime_error("login no password"); 
             }
            break;
        };
        case MessageType::PrivateChat:
        {
      
          if(j["senderId"].get<uint32_t>() == 0)
        {
            throw std::runtime_error(
                "senderId invalid");
        }
         if(!j["payload"].contains("text"))
        {
            throw std::runtime_error("chat no text");
        }

            
        }  

        default:
        break;
    }
}
Message JsonCodec::buildMessage(nlohmann::json& j,MessageType type)
{
  Message msg;
  msg.setType(type);

    msg.setSequence( j["sequence"]);

    msg.setSenderId( j["senderId"]);

    msg.setReceiverId(j["receiverId"]);

    msg.setTimestamp( j["timestamp"]);

    msg.setPayload(j["payload"]);

    return msg;
}
Message JsonCodec::decode(const std::string& data)
{
    json j = parseJson(data);

    dataRequireFields( j);

    dataFieldTypes( j);

    MessageType type = parseMessageType( j);

    dataMessage(j, type);

    return buildMessage(j, type);
}