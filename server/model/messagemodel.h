#pragma once
#include <vector>
#include "entity/message.h"
#include <mysql/mysql.h>
#include <optional>
#include "../database/mysql/mysqlresult.h"
class MessageModel
{
   public:
   bool insert(ChatMessage& message);
   std::optional<ChatMessage> findById(std::int64_t id);
   std::vector<ChatMessage> findPriHistory(int userid, int peerid,size_t limit,size_t offset);
   std::vector<ChatMessage> findGroupHistory(int groupid,size_t limit,size_t offset);
   std::vector<ChatMessage> findUserGroupHistory(int userid, int groupid, size_t limit, size_t offset);


   private:
   ChatMessage makeMessage(MysqlResult& result);

};