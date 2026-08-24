#include "packetcodec.h"
#include "../buffer.h"
#include<arpa/inet.h>
#include <cstring>
#include <iostream>
 std::string PacketCodec:: encode(const std::string& json)
 {
    return encode(json,nullptr,0);
 }

std::string PacketCodec::encode(const std::string& json,const void* body,size_t bodyLen)
{
    PacketHeader header;

    header.length=htonl(static_cast<uint32_t>(json.size()));
    header.bodylength=htonl(static_cast<uint32_t>(bodyLen));
    header.magic=htons(kMagic);
    header.version=htons(kVersion);

    std::string packet;
    packet.append(reinterpret_cast<const char*>(&header),sizeof(header));
    packet.append(json);

    if(body && bodyLen>0)
    {
        packet.append(static_cast<const char*>(body),bodyLen);
    }
  
    return packet;

}
PacketCodec::DecodeResult PacketCodec::decode(Buffer& buffer,std::string& json,std::string *body)
{
    if(buffer.readableBytes()<kHeaderSize)
    {
         return DecodeResult::Needmoredata;
    }
    PacketHeader header;

    std::memcpy(&header,buffer.peek(),kHeaderSize);
    header.length=ntohl(header.length);
    header.bodylength=ntohl(header.bodylength);
    header.magic=ntohs(header.magic);
    header.version=ntohs(header.version);////

    if(header.magic!=kMagic)
    {
        return DecodeResult::ProtocolError;
    }
    if(header.version!= kVersion)
    {
        return DecodeResult::ProtocolError;
    }
    if(buffer.readableBytes()<kHeaderSize+header.length+header.bodylength)
    {
        return DecodeResult::Needmoredata;

    }
    if (header.length > kMaxPayloadSize)
   {
       return DecodeResult::ProtocolError;
   }
    if (header.bodylength > kMaxBodySize)
   {
       return DecodeResult::ProtocolError;
   }
    json.assign(buffer.peek()+kHeaderSize,header.length);//先取json元数据
    buffer.retrieve(kHeaderSize+header.length);

    if(body)
    {
    body->assign(buffer.peek(),header.bodylength);
    }
   buffer.retrieve(header.bodylength);
    return DecodeResult::Ok;
}