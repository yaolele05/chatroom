#include <iostream>

#include "../server/model/usermodel.h"
#include "../server/business/service/loginservice.h"
#include "../server/database/connectionpool/mysqlpool.h"
#include "../server/database/connectionpool/redispool.h"
#include "../server/session/usersession.h"

int main()
{
    if(!MysqlPool::instance().init(
        "127.0.0.1",
        3306,
        "chatserver",
        "123456",
        "chatroom",
        2))
    {
      std::cout<<"mysql init failed\n";
    return -1;

     }
    if(!RedisPool::instance().init(
        "127.0.0.1",
        6379,
        2))
    {
      std::cout<<"redis init failed\n";
    return -1;
    }

    User user;
    user.setUsername("tom");
    user.setPasswordHash("123456");
    user.setNickname("Tom");
    user.setAvatar("avatar.png");
    user.setSignature("hello");

    UserModel model;

    if (!model.findByName("tom"))
  {
    if(!model.insert(user))
    {
        std::cout<<"insert user failed\n";
        return -1;
    }
  } 

    auto session = std::make_shared<UserSession>();

    LoginService& service = LoginService::instance();
    bool ok = service.login(session, "tom", "123456");

    if (ok)
    {
        std::cout << "login success\n";
        std::cout << "userid = " << session->userid() << '\n';
        std::cout << "username = " << session->username() << '\n';
        std::cout<<"authenticated="<<session->authenticated()<<std::endl;

        auto redis = RedisPool::instance().getConnection();
        if(!redis)
    {
    std::cout<<"redis connection failed\n";
    return -1;
    }
     if(redis)
    {
    bool online = redis->isUserOnline(session->userid());

        std::cout 
    << "online="
    << online
    << std::endl;


     auto token = redis->getToken(session->userid());

    if(token)
   {
    std::cout 
        << "token="
        << *token
        << std::endl;
    }
    RedisPool::instance()
       .releaseConnection(redis);
    }

    service.logout(session);

    auto redis2 =
            RedisPool::instance().getConnection();


        if(redis2)
        {
            bool online =
                redis2->isUserOnline(
                    session->userid()
                );


            std::cout
                <<"after logout online="
                <<online
                <<std::endl;
            std::cout
      <<"after logout authenticated="
     <<session->authenticated()
      <<std::endl;

            RedisPool::instance()
            .releaseConnection(redis2);
        }

    }
    else
    {
        std::cout << "login failed\n";
    }
    

   
    return 0;
}