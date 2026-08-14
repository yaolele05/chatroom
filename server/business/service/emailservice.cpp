#include "emailservice.h"
#include <cerrno>
#include <iostream>
#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <cstdlib>
EmailService& EmailService::instance()
{
    static EmailService instance;
      return instance;
}
bool EmailService::send(const std::string& to,const std::string& subject,const std::string& body)
{
    const char* username=std::getenv("CHATROOM_SMTP_USER");
    const char* password=std::getenv("CHATROOM_SMTP_PASSWORD");
    std::cout << "[EmailService] username="
          << (username ? username : "NULL")
          << std::endl;

    std::cout << "[EmailService] password="
          << (password ? "SET" : "NULL")
          << std::endl;
    if(username==nullptr || password==nullptr)
    {
        std::cerr<<"[EmailService]SMTP environment "<<"variables not configured"<<std::endl;
        return false;
    }
    username_=username;
    password_=password;
    CURL* curl=curl_easy_init();
    if(curl==nullptr)
    {
        std::cout<<"[EmailService] curl init failed"<<std::endl;
        return false;
    }

   curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_PROXY, "");
    std::string from="<"+username_+">";
    std::string recipient="<"+to+">";
    std::string mail;

    mail+="From:ChatRoom <"+username_+">\r\n";
    mail += "To: <" + to + ">\r\n";
    mail += "Subject: " + subject + "\r\n";
    mail += "MIME-Version: 1.0\r\n";
    mail += "Content-Type: text/plain; charset=UTF-8\r\n";
    mail += "\r\n";
    mail += body;
    mail += "\r\n";

    struct UploadStatus
    {
        const std::string* data;
        size_t position;
    };
    UploadStatus uploadStatus{&mail,0};
    curl_easy_setopt(curl,CURLOPT_URL,smtpServer_.c_str());
    curl_easy_setopt(curl,CURLOPT_USERNAME,username_.c_str());
    curl_easy_setopt(curl,CURLOPT_PASSWORD,password_.c_str());
    curl_easy_setopt(curl, CURLOPT_LOGIN_OPTIONS, "AUTH=LOGIN");
    curl_easy_setopt( curl,CURLOPT_MAIL_FROM,from.c_str());
    struct curl_slist* recipients = nullptr;
    recipients = curl_slist_append(recipients,recipient.c_str());
    curl_easy_setopt(curl,CURLOPT_MAIL_RCPT,recipients);
    curl_easy_setopt(curl,CURLOPT_USE_SSL,CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
    curl_easy_setopt(curl,CURLOPT_UPLOAD,1L);
    curl_easy_setopt(curl,CURLOPT_READFUNCTION,+[](char* buffer,size_t size, size_t nitems,void* userdata) -> size_t
        {
            UploadStatus* upload =static_cast<UploadStatus*>(userdata);
            size_t bufferSize =size * nitems;
            size_t remaining =upload->data->size()- upload->position;
            size_t copySize =remaining < bufferSize? remaining : bufferSize;

            if(copySize > 0)
            {
                std::memcpy(buffer, upload->data->data()+ upload->position,copySize);
                upload->position += copySize;
            }

            return copySize;
        });

    curl_easy_setopt(curl,CURLOPT_READDATA,&uploadStatus);
    curl_easy_setopt(curl,CURLOPT_TIMEOUT,15L);

    CURLcode result =curl_easy_perform(curl);
    bool success =result == CURLE_OK;

    if(!success)
    {
        std::cerr<< "[EmailService] send failed: "<< curl_easy_strerror(result)<< std::endl;
    }
    else
    {
        std::cout<< "[EmailService] send success"<< " to=" << to << std::endl;
    }

    curl_slist_free_all(recipients);

    curl_easy_cleanup(curl);

    return success;
}