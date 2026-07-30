#pragma once
#include <vector>
#include "entity/message.h"
class MessageModel
{
   public:
   bool insert(const Message& message);
   std::vector<Message> queryPriHistory(int userid, int peerid);
   std::vector<Message> queryGroupHistory(int userid,int groupid);
   bool removePriHistory(int userid,int peerid);
   bool removeGroupHistory(int groupid);

};