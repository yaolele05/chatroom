#pragma once
#include <string>
#include <memory>
#include <optional>
#include "../../model/usermodel.h"
#include "../../database/redis/redisclient.h"

#include "../../session/session.h"
#include "../../session/usersession.h"
#include "../../session/sessionmanager.h"
#include "../../../common/protocol/message.h"
class UserSession;

class LoginService
{
    public:
    static LoginService& instance();
    static void registerHandle();
    void registerUser(const Message& msg,Session* se);

    void login(const Message& msg,Session* se );
    void logout(const Message& msg,Session* se);


    private:

    LoginService()=default;
    LoginService(const LoginService&) = delete;
    LoginService& operator=(const LoginService&) = delete;

};