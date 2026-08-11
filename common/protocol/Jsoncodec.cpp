#include "Jsoncodec.h"
#include <stdexcept>

using json = nlohmann::json;
std::string JsonCodec::TypeToString(Messagetype type)
{
    switch(type)
    {
     case Messagetype::Register:return "register";
     case Messagetype::Login:return "login";
     case Messagetype::Logout:return "logout";
     case Messagetype::RegisterResponse:return "register_response";
     case Messagetype::LoginResponse:return "login_response";
     case Messagetype::LogoutResponse:return "logout_response";
     case Messagetype::AddFriend:return "add_friend";
     case Messagetype::AddFriendResponse:return "addfriend_response";
     case Messagetype::DeleteFriend:return "delete_friend";
     case Messagetype::DeleteFriendResponse:return "deletefriend_response";
     case Messagetype::FriendList:return "friend_list";
     case Messagetype::FriendListResponse:return "friendlist_response";
     case Messagetype::FriendOnlineStatus:return "friend_online_status";
    
     case Messagetype::PrivateChat:return "private_chat";
    case Messagetype::PrivateChatResponse:return "privatechat_response";
     case Messagetype::CreateGroup:return "create_group";
     case Messagetype::CreateGroupResponse: return "creategroup_response";

     case Messagetype::JoinGroup:return "join_group";
     case Messagetype::JoinGroupResponse:return "joingroup_response";
    case Messagetype::LeaveGroup: return"leave_group";
     case Messagetype::LeaveGroupResponse:return "leavegroup_response";
     case Messagetype::GroupChat:return "group_chat";
     case Messagetype::GroupChatResponse:return "groupchat_response";
     case Messagetype::GroupList:
     return "group_list";
     case Messagetype::GroupListResponse:
     return "grouplist_response";

     
     case Messagetype::HeartBeat: return "heartbeat";
     case Messagetype::HeartBeatResponse:
    return "heartbeat_response";

    case Messagetype::FileStart: return "file_start";
    case Messagetype::GroupFileStart: return"groupfile_start";
    case Messagetype::FileChunk: return "file_chunk";
    case Messagetype::FileFinish: return "file_finish";

    case Messagetype::FILE_ACK: return "file_ack";
    case Messagetype::FILE_RESUME_REQUEST: return "file_resume_request";
    case Messagetype::FILE_RESUME_RESPONSE: return "file_resume_response";

    case Messagetype::MessageAck: return "message_ack";
    case Messagetype::MessageRecall:return "message_recall";
    case Messagetype::MessageRead: return "message_read";
    case Messagetype::Error: return "error";
    case Messagetype::HistoryRequest:return "history_request";
    case Messagetype::HistoryResponse:return "history_response";
    case Messagetype::OfflineFileNotify:return "offlinefile_notify";
    case Messagetype::FileDownloadRequest:return "filedownload_request";
  
    default: return"unknown";
    }
}
Messagetype JsonCodec::StringToType(const std::string& type)
{
     if(type=="register")
        return Messagetype::Register;
    if(type=="login")
        return Messagetype::Login;

    if(type=="logout")
        return Messagetype::Logout;
    if(type=="register_response")
        return Messagetype::RegisterResponse;
    if(type=="login_response")
        return Messagetype::LoginResponse;
    if(type=="logout_response")
       return Messagetype::LogoutResponse;

    if(type=="add_friend")
        return Messagetype::AddFriend;
    if(type=="addfriend_response")
       return Messagetype::AddFriendResponse;
       
    if(type=="delete_friend")
        return Messagetype::DeleteFriend;
    if(type=="deletefriend_response")
       return Messagetype::DeleteFriendResponse;
       
    if(type=="friend_list")
        return Messagetype::FriendList;
    if(type=="friendlist_response")
       return Messagetype::FriendListResponse;
       
    if(type=="friend_online_status")
        return Messagetype::FriendOnlineStatus;

    if(type=="private_chat")
        return Messagetype::PrivateChat;
    if(type=="privatechat_response")
       return Messagetype::PrivateChatResponse;
       
    if(type=="create_group")
        return Messagetype::CreateGroup;
    if(type=="creategroup_response")
       return Messagetype::CreateGroupResponse;
       
    if(type=="join_group")
        return Messagetype::JoinGroup;
    if(type=="joingroup_response")
       return Messagetype::JoinGroupResponse;
       
    if(type=="leave_group")
        return Messagetype::LeaveGroup;
    if(type=="leavegroup_response")
       return Messagetype::LeaveGroupResponse;
       
    if(type=="group_chat")
        return Messagetype::GroupChat;
    if(type=="groupchat_response")
       return Messagetype::GroupChatResponse;
    if(type == "group_list")
    return Messagetype::GroupList;
    if(type == "grouplist_response")
    return Messagetype::GroupListResponse;
    if(type=="heartbeat")
        return Messagetype::HeartBeat;
     if(type == "heartbeat_response")
    return Messagetype::HeartBeatResponse;
    if(type=="file_start")
    return Messagetype::FileStart;
    if(type=="groupfile_start")
    return Messagetype::GroupFileStart;
    if(type=="file_chunk")
    return Messagetype::FileChunk;

    if(type=="file_finish")
        return Messagetype::FileFinish;
    if(type=="file_ack")
    return Messagetype::FILE_ACK;
    if(type=="file_resume_request")
    return Messagetype::FILE_RESUME_REQUEST;
    if(type=="file_resume_response")
    return Messagetype::FILE_RESUME_RESPONSE;
    
    if(type=="message_ack")
        return Messagetype::MessageAck;
    if(type=="message_recall")
        return Messagetype::MessageRecall;

    if(type=="message_read")
        return Messagetype::MessageRead;
    if(type=="error")
        return Messagetype::Error;
    if(type=="history_request")
      return Messagetype::HistoryRequest;
    if(type=="history_response")
      return Messagetype::HistoryResponse;

      if(type=="offlinefile_notify")
      return Messagetype::OfflineFileNotify;
      if(type=="filedownload_request")
      return Messagetype::FileDownloadRequest;

    return Messagetype::unknown;
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
    static constexpr const char* fields[]={  "type","sequence","senderId","receiverId","timestamp", "payload"};
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
Messagetype JsonCodec::parseMessageType(const nlohmann::json& j)
{
    Messagetype type=StringToType(j.at("type").get<std::string>());
    if(type == Messagetype::unknown)
    { 
     throw std::runtime_error("unknown Message type");
    }
    return type;
}
void JsonCodec::dataMessage(nlohmann::json& j,Messagetype type)
{
    switch(type)
    {
        case Messagetype::Login:
        {
            if(!j["payload"].contains("username"))
            {
                throw std::runtime_error("login no username");
            }
             if(!j["payload"].contains("password"))
             {
               throw std::runtime_error("login no password"); 
             }
            break;
        };
        case Messagetype::PrivateChat:
        {
      
         if(!j["payload"].contains("content"))
        {
            throw std::runtime_error("chat no content");
        } 
        }  

        default:
        break;
    }
}
Message JsonCodec::buildMessage(nlohmann::json& j,Messagetype type)
{
  Message msg;
  msg.setType(type);

    msg.setSequence( j["sequence"].get<uint64_t>());

    msg.setSenderId( j["senderId"].get<uint64_t>());//

    msg.setReceiverId(j["receiverId"].get<uint64_t>());//

    msg.setTimestamp( j["timestamp"].get<uint64_t>());

    msg.setPayload(j["payload"]);

    return msg;
}
Message JsonCodec::decode(const std::string& data)
{
    json j = parseJson(data);

    dataRequireFields( j);

    dataFieldTypes( j);

    Messagetype type = parseMessageType( j);

    dataMessage(j, type);

    return buildMessage(j, type);
}