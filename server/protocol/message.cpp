#include "message.h"

Message::Message()
{

}
Message::Message(MessageType type):type_(type)
{

}
void Message::setType(MessageType type)
{
    type_=type;
}
MessageType Message::type() const
{
     return type_;
}
void Message::setSequence(uint64_t seq)
{
    sequence_=seq;
}
uint64_t Message:: sequence() const
{
     return sequence_;
}
void Message::setSenderId(uint32_t id)
{
    senderId_=id;   
}
uint32_t Message::senderId() const
{
    return senderId_;
}
void Message::setReceiverId(uint32_t id)
{
   receiverId_=id;
}
uint32_t Message::receiverId() const
{
    return receiverId_;
}
void Message::setTimestamp(uint64_t id)
{
   timestamp_=id;
}
uint64_t Message::timestamp() const
{
    return timestamp_;
}
void Message::setPayload(const nlohmann::json& payload) 
{
    payload_=payload;
}
const nlohmann::json& Message:: payload() const
{
  return payload_;
}
nlohmann::json& Message::payload() 
{
   return payload_;
}
