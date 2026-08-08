#include "sha256.h"
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <fstream>
#include <iomanip>
#include <sstream>
std::string Sha256::data(const std::string& text)
{
    std::vector<uint8_t> bytes(text.begin(),text.end());

    return memory(bytes);
}
std::string Sha256::memory(const std::vector<uint8_t>& data)
{
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLen = 0;
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if(!ctx)
      return {};
    EVP_DigestInit_ex(ctx,EVP_sha256(),nullptr);
    EVP_DigestUpdate(ctx,data.data(),data.size());
    EVP_DigestFinal_ex( ctx, hash,&hashLen);


    EVP_MD_CTX_free(ctx);
    std::stringstream ss;
    for(unsigned int i=0;i<hashLen;i++)
    {
        ss<< std::hex<< std::setw(2)<< std::setfill('0')<< static_cast<int>(hash[i]);
    }
    return ss.str();
}
std::string Sha256::file(const std::string& path)
{
    std::ifstream ifs(path,std::ios::binary);
    if(!ifs)
    return {};
    EVP_MD_CTX* ctx =EVP_MD_CTX_new();
    if(!ctx)
        return {};


    EVP_DigestInit_ex(
        ctx,
        EVP_sha256(),
        nullptr
    );


    char buffer[8192];


    while(ifs)
    {
        ifs.read(
            buffer,
            sizeof(buffer)
        );


        std::streamsize n =
            ifs.gcount();


        if(n>0)
        {
            EVP_DigestUpdate(
                ctx,
                buffer,
                n
            );
        }
    }


    unsigned char hash[EVP_MAX_MD_SIZE];

    unsigned int hashLen=0;


    EVP_DigestFinal_ex(
        ctx,
        hash,
        &hashLen
    );


    EVP_MD_CTX_free(ctx);


    std::stringstream ss;

    for(unsigned int i=0;i<hashLen;i++)
    {
        ss
        <<std::hex
        <<std::setw(2)
        <<std::setfill('0')
        <<static_cast<int>(hash[i]);
    }


    return ss.str();
}