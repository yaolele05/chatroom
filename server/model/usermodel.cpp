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
    user.setEmail(result.get<std::string>(3));
    user.setNickname(result.get<std::string>(4));
    user.setAvatar(result.get<std::string>(5));
    user.setSignature(result.get<std::string>(6));
 
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
        username,password_hash,email,nickname,avatar,signature)VALUES(?,?,?,?,?,?)
    )");

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
    return false;
    }
    stmt->bind(0,user.username());
    stmt->bind(1,user.passwordHash());
    stmt->bind(2,user.email());
    stmt->bind(3,user.nickname());
    stmt->bind(4,user.avatar());
    stmt->bind(5,user.signature());

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
    nickname=?, avatar=?, signature=?, password_hash=? WHERE id=?)");

    if(!stmt)
    {
         MysqlPool::instance().releaseConnection(conn);
    return false;
    }
    stmt->bind(0,user.nickname());
    stmt->bind(1,user.avatar());
    stmt->bind(2,user.signature());
    stmt->bind(3,user.passwordHash());
    stmt->bind(4,user.id());
   
     bool ok=stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;
}
bool UserModel::remove(int userid)
{

    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    return false;

    auto stmt=conn->prepare(R"(DELETE FROM users WHERE id=?)");

    if(!stmt)
    {
         MysqlPool::instance().releaseConnection(conn);
    return false;
    }
    stmt->bind(0,userid);

    bool ok=stmt->execute();
    MysqlPool::instance().releaseConnection(conn);
    return ok;

}
std::optional<User> UserModel::findById(int userid)
{
  auto conn=MysqlPool::instance().getConnection();
  if(!conn)
  return std::nullopt;

  auto stmt=conn->prepare("SELECT " "id,"  "username,"   "password_hash,"  "email," "nickname,"  "avatar,"  "signature "
   "FROM users "  "WHERE id=?");

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
     
    auto conn=MysqlPool::instance().getConnection();
    if(!conn)
    {

        std::cout<<"get mysql connection failed"<<std::endl;
        return std::nullopt;
    }

    std::cout<<"get mysql connection ok"<<std::endl;
    auto stmt=conn->prepare("SELECT "  "id,"  "username,"  "password_hash,"  "email,"    "nickname,"   "avatar,"    "signature "     "FROM users "   "WHERE username=?");
    if(!stmt)
    {
            std::cout<<"prepare failed"<<std::endl;
        MysqlPool::instance().releaseConnection(conn);
        return std::nullopt;
    }
  
    stmt->bind(0,username);
    
    auto result=stmt->query();
   std::cout<<"query returned"<<std::endl;
   
    if(result.fetch())
    {
        
        auto user=makeUser(result);
        MysqlPool::instance().releaseConnection(conn);
        return user;
    }
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
    auto stmt=conn->prepare("SELECT "  "id,"  "username,"   "password_hash,"   "email," "nickname,"   "avatar,"  "signature "  "FROM users " );
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
 std::optional<User> UserModel::findByEmail(const std::string& email)
{

    auto conn = MysqlPool::instance().getConnection();
    if(!conn)
    {
        std::cout << "get mysql connection failed" << std::endl;
        return std::nullopt;
    }

    auto stmt = conn->prepare(  "SELECT " "id,"  "username," "password_hash,"  "email," "nickname," "avatar,"   "signature "
        "FROM users " "WHERE email=?");

    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return std::nullopt;
    }

    stmt->bind(0, email);
    auto result = stmt->query();
    if(result.fetch())
    {
        auto user = makeUser(result);
        MysqlPool::instance().releaseConnection(conn);
        return user;
    }

    MysqlPool::instance().releaseConnection(conn);
    return std::nullopt;
}
bool UserModel::updatePassword(int userid,const std::string& passwordHash)
{
    auto conn = MysqlPool::instance().getConnection();
    if(!conn)
        return false;
    auto stmt = conn->prepare(  "UPDATE users "  "SET password_hash=? "  "WHERE id=?"    );
    if(!stmt)
    {
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    stmt->bind(0, passwordHash);
    stmt->bind(1, userid);
    bool ok = stmt->execute();

    MysqlPool::instance().releaseConnection(conn);

    return ok;
}
bool UserModel::deleteAccount(int userid)
{
    auto conn = MysqlPool::instance().getConnection();

    if(!conn)
    {
        std::cerr << "[UserModel] get mysql connection failed\n";
        return false;
    }
    if(!conn->beginTransaction())
    {
        std::cerr << "[UserModel] begin transaction failed: "<< conn->error() << std::endl;
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }

    auto rollback = [&]()
    {   conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
    };

    {
        auto stmt = conn->prepare("DELETE FROM offline_message "   "WHERE user_id=?");
        if(!stmt)
        {
            rollback();
            return false;
        }
        stmt->bind(0, userid);
        if(!stmt->execute())
        {
            rollback();
            return false;
        }
    }

    {
        auto stmt = conn->prepare( "DELETE FROM message "  "WHERE sender_id=? OR receiver_id=?");
        if(!stmt)
        {
            rollback();
            return false;
        }

        stmt->bind(0, userid);
        stmt->bind(1, userid);

        if(!stmt->execute())
        {
            rollback();
            return false;
        }
    }
    {
        auto stmt = conn->prepare( "DELETE FROM file_receiver "  "WHERE user_id=?");

        if(!stmt)
        {
            rollback();
            return false;
        }
        stmt->bind(0, userid);
        if(!stmt->execute())
        {
            rollback();
            return false;
        }
    }

    {
        auto stmt = conn->prepare( "DELETE FROM file " "WHERE sender_id=? OR receiver_id=?");
        if(!stmt)
        {
            rollback();
            return false;
        }

        stmt->bind(0, userid);
        stmt->bind(1, userid);

        if(!stmt->execute())
        {
            rollback();
            return false;
        }
    }

    {
        auto stmt = conn->prepare( "DELETE FROM friend_request " "WHERE from_user_id=? OR to_user_id=?");
        if(!stmt)
        {
            rollback();
            return false;
        }

        stmt->bind(0, userid);
        stmt->bind(1, userid);

        if(!stmt->execute())
        {
            rollback();
            return false;
        }
    }

    {
        auto stmt = conn->prepare(  "DELETE FROM user_friend " "WHERE user_id=? OR friend_id=?");
        if(!stmt)
        {
            rollback();
            return false;
        }

        stmt->bind(0, userid);
        stmt->bind(1, userid);
        if(!stmt->execute())
        {
            rollback();
            return false;
        }
    }

    {
        auto stmt = conn->prepare(  "DELETE FROM group_join_request "   "WHERE  from_user_id=?");
        if(!stmt)
        {
            rollback();
            return false;
        }
        stmt->bind(0, userid);
        if(!stmt->execute())
        {
            rollback();
            return false;
        }
    }

    {
        auto stmt = conn->prepare(  "DELETE FROM chatgroup_member "  "WHERE user_id=?");
        if(!stmt)
        {
            rollback();
            return false;
        }
        stmt->bind(0, userid);
        if(!stmt->execute())
        {
            rollback();
            return false;
        }
    }

    {
        auto stmt = conn->prepare( "DELETE FROM users "  "WHERE id=?");
        if(!stmt)
        {
            rollback();
            return false;
        }
        stmt->bind(0, userid);
        if(!stmt->execute())
        {
            rollback();
            return false;
        }
    }

    if(!conn->commit())
    {
        std::cerr << "[UserModel] commit failed: "<< conn->error() << std::endl;

        conn->rollback();
        MysqlPool::instance().releaseConnection(conn);
        return false;
    }
    MysqlPool::instance().releaseConnection(conn);
    std::cout << "[UserModel] delete account success, userid=" << userid << std::endl;

    return true;
}
