#include "usermodel.h"
#include "../database/connectionpool/mysqlpool.h"
#include "../database/mysql/mysqlclient.h"
#include "../database/mysql/mysqlresult.h"
#include <iostream>
User UserModel::makeUser(const MysqlResult& result)
{
    User user;
    user.setId(result.get<int>(0));
    user.setUsername(result.get<std::string>(1));
    user.setPasswordHash(result.get<std::string>(2));
    user.setNickname(result.get<std::string>(3));
    user.setAvatar(result.get<std::string>(4));
    user.setSignature(result.get<std::string>(5));
  /*  user.setCreateTime(result.get<std::chrono::system_clock::time_point>(7));
    user.setUpdateTime(result.get<std::chrono::system_clock::time_point>(8));
   */
    return user;
}
bool UserModel::insert( User& user)
{
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return false;
    }
    auto stmt=conn->prepare(R"(INSERT INTO users(
        username,password_hash,nickname,avatar,signature)VALUES(?,?,?,?,?)
    )");

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
    return false;
    }
    stmt->bind(0,user.username());
    stmt->bind(1,user.passwordHash());
    stmt->bind(2,user.nickname());
    stmt->bind(3,user.avatar());
    stmt->bind(4,user.signature());

    if(!stmt->execute())
    {

        return false;

    }

    user.setId(static_cast<int>(conn->lastInsertId()));
    return true;
}
bool UserModel::update(const User& user)
{
   auto conn=MysqlPool::instance().getConnection();
   if(!conn)
   {
    return false;
   }
   
   auto stmt=conn->prepare(R"(UPDATE users SET
    nickname=?,avatar=?,signature=?,password_hash=?WHERE id=?)");

    if(!stmt)
    {
         MysqlPool::instance().getConnection();
    return false;
    }
    stmt->bind(0,user.nickname());
    stmt->bind(1,user.avatar());
    stmt->bind(2,user.signature());
    stmt->bind(3,user.passwordHash());
    stmt->bind(4,user.id());
   
    return stmt->execute();
}
bool UserModel::remove(int userid)
{

    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    return false;

    auto stmt=conn->prepare(R"(DELETE FROM users WHERE id=?)");

    if(!stmt)
    {
         MysqlPool::instance().getConnection();
    return false;
    }
    stmt->bind(0,userid);

    return stmt->execute();

}
std::optional<User> UserModel::findById(int userid)
{
  auto conn=MysqlPool::instance().getConnection();
  if(!conn)
  return std::nullopt;

  auto stmt=conn->prepare("SELECT "
   "id,"
   "username,"
   "password_hash,"
   "nickname,"
   "avatar,"
   "signature "
   "FROM users "
   "WHERE id=?"
  );

  if(!stmt)
  {
    MysqlPool::instance().releaseConnection(conn);
    return std::nullopt;

  }

  stmt->bind(0,userid);
  auto result=stmt->query();
  if(result.fetch())
  {
    auto user=makeUser(result);

    MysqlPool::instance().releaseConnection(conn);
    return user;
  }

  MysqlPool::instance().releaseConnection(conn);
  return std::nullopt;
}
std::optional<User> UserModel::findByName(const std::string& username)
{
     std::cout<<"findByName username="<<username<<std::endl;
     std::cout<<"before get mysql connection"<<std::endl;
    auto conn=MysqlPool::instance().getConnection();
    std::cout<<"after get mysql connection"<<std::endl;

    if(!conn)
    {

        std::cout<<"get mysql connection failed"<<std::endl;
        return std::nullopt;
    }

    std::cout<<"get mysql connection ok"<<std::endl;
    auto stmt=conn->prepare("SELECT " 

        "id,"
        "username,"
         "password_hash,"
          "nickname,"
           "avatar,"
           "signature " 
           "FROM users "
            "WHERE username=?");

    if(!stmt)
    {
            std::cout<<"prepare failed"<<std::endl;
        MysqlPool::instance().releaseConnection(conn);
        return std::nullopt;
    }
    std::cout<<"prepare ok"<<std::endl;
    stmt->bind(0,username);
     
    std::cout<<"bind ok username="
             <<username
             <<std::endl;
    auto result=stmt->query();
        std::cout<<"query returned"<<std::endl;
   
    if(result.fetch())
    {
         std::cout<<"fetch success"<<std::endl;
        auto user=makeUser(result);

        std::cout<<"find user id="
             <<user.id()
             <<" name="
             <<user.username()
             <<std::endl;
        MysqlPool::instance().releaseConnection(conn);
        return user;
    }
    std::cout<<"fetch empty"<<std::endl;
    MysqlPool::instance().releaseConnection(conn);
    return std::nullopt;
}
std::vector<User> UserModel::findAll()
{
    std::vector<User> users;
     auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {
        return users;
    }
    auto stmt=conn->prepare("SELECT " 
        "id,"
        "username,"
         "password_hash,"
          "nickname,"
           "avatar,"
           "signature " 
           "FROM users "
           );

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return users;
    }

    auto result=stmt->query();
    while(result.fetch())
    {
        users.push_back(makeUser(result));
    }

    MysqlPool::instance().releaseConnection(conn);

    return users;

}