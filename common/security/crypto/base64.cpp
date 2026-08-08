#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include "base64.h"
std::string Base64::encode(const std::vector<unsigned char>& data )
{

    BIO* b64=BIO_new(BIO_f_base64());
    BIO* mem=BIO_new(BIO_s_mem());
    BIO_push(b64, mem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64,data.data(),data.size());
    BIO_flush(b64);
    BUF_MEM* buffer;
    BIO_get_mem_ptr(b64,&buffer);
    std::string result(buffer->data,buffer->length);
    BIO_free_all(b64);
    return result;

}
std::vector<unsigned char> Base64:: decode(const std::string& text)
{
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO* mem =BIO_new_mem_buf(text.data(),text.size());
    BIO_push(b64, mem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    std::vector<unsigned char> out(text.size());

    int len =BIO_read(b64,out.data(), out.size());
    out.resize(len);
    BIO_free_all(b64);
    return out;
}