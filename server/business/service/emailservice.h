#pragma once
#include <string>

class EmailService
{
    public:
    static EmailService& instance();
    bool send(const std::string& to,const std::string& subject,const std::string& body);

    private:
    EmailService();
    EmailService(const EmailService&)=delete;
    EmailService& operator=(const EmailService& )=delete;

    private:
    std::string smtpServer_= "smtps://smtp.163.com:465";
    std::string username_= "18220473526@163.com";
    std::string password_ = "JMqUirneknSEgeb3";


};
