#include "packetcodec.h"
#include "../buffer.h"
#include<arpa/inet.h>
#include <cstring>
#include <iostream>
std::string PacketCodec::encode(const std::string& json)
{
    PacketHeader header;

    header.length=htonl(static_cast<uint32_t>(json.size()));
    header.magic=htons(kMagic);
    header.version=htons(kVersion);

    std::string packet;
    packet.append(reinterpret_cast<const char*>(&header),sizeof(header));
    packet.append(json);
    //std::cout<<"encode header size=" <<sizeof(PacketHeader) <<std::endl;
    return packet;

}
PacketCodec::DecodeResult PacketCodec::decode(Buffer& buffer,std::string& json)
{
    if(buffer.readableBytes()<kHeaderSize)
    {
         return DecodeResult::Needmoredata;
    }
    PacketHeader header;

    std::memcpy(&header,buffer.peek(),kHeaderSize);
    header.length=ntohl(header.length);
    header.magic=ntohs(header.magic);
    header.version=ntohs(header.version);

    if(header.magic!=kMagic)
    {
        return DecodeResult::ProtocolError;
    }
    if(header.version!= kVersion)
    {
        return DecodeResult::ProtocolError;
    }
    if(buffer.readableBytes()<kHeaderSize+header.length)
    {
        return DecodeResult::Needmoredata;

    }
    if (header.length > kMaxPayloadSize)
   {
       return DecodeResult::ProtocolError;
   }
    json.assign(buffer.peek()+kHeaderSize,header.length);
    buffer.retrieve(kHeaderSize+header.length);

    //std::cout<<"decode header size=" <<sizeof(PacketHeader)  <<std::endl;
    return DecodeResult::Ok;
}