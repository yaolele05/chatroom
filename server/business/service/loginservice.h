#pragma once
#include <string>
#include <memory>
#include <optional>
#include "../../model/usermodel.h"
#include "../../database/redis/redisclient.h"
class UserSession;

class LoginService
{
    public:
    static LoginService& instance();
    bool login(const std::shared_ptr<UserSession>& session,const std::string& username, const std::string& password);
    bool logout(const std::shared_ptr<UserSession>& session);


    private:

    LoginService()=default;
   

};