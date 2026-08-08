#pragma once

#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>

#include "messagetype.h"
class Message
{
    public:
    Message();
    Message(Messagetype type);

    Messagetype type() const;
    void setType(Messagetype type);

    uint64_t sequence() const;
    void setSequence(uint64_t seq );

    uint32_t senderId()const;
    void setSenderId(uint32_t id);   
    
    uint32_t receiverId() const;
    void setReceiverId(uint32_t id );

    uint64_t timestamp() const;
    void setTimestamp(uint64_t ts);


    const nlohmann::json& payload() const;
    nlohmann::json& payload();

    void setPayload(const nlohmann::json& payload);


    private:
    
   
    Messagetype type_{Messagetype::unknown};
    uint64_t sequence_{0};
    uint32_t senderId_{0};
    uint32_t receiverId_{0};
    uint64_t timestamp_{0};

    nlohmann::json payload_;

};