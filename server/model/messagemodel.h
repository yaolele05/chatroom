#pragma once
#include <vector>
#include "entity/message.h"
#include <mysql/mysql.h>
#include <optional>
#include "../database/mysql/mysqlresult.h"
class MessageModel
{
   public:
   bool insert(Message& message);
   std::optional<Message> findById(std::int64_t id);
   std::vector<Message> findPriHistory(int userid, int peerid,size_t limit,size_t offset);
   std::vector<Message> findGroupHistory(int groupid,size_t limit,size_t offset);
   std::vector<Message> findUserGroupHistory(int userid, int groupid, size_t limit, size_t offset);


   private:
   Message makeMessage(MysqlResult& result);

};