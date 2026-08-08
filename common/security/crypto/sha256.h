#pragma once

#include <string>
#include <cstdint>
#include<vector>
class Sha256
{
public:

    // 文件SHA256
    static std::string file(const std::string& path);
    static std::string data(const std::string& text);
    //二进制
    static std::string memory( const std::vector<uint8_t>& data);    

};