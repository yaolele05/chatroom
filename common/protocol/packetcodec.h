#pragma once 
#include <cstdint>
#include <string>

class Buffer;

class PacketCodec
{
    public:

    enum class DecodeResult
    {
        Ok,Needmoredata,ProtocolError
    };

    static std::string encode(const std::string& json);
    static DecodeResult decode(Buffer& buffer,std::string& json);

    private:
    struct PacketHeader
    {

        uint32_t length;
        uint16_t magic;
        uint16_t version;
    };

    static const uint16_t kMagic=0xCAFE;
    static const  uint16_t kVersion=1;
    static  const uint32_t kHeaderSize=8;
    static const  uint32_t kMaxPayloadSize = 1*1024* 1024; 
};