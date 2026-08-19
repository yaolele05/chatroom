#pragma once
#include <vector>
#include "entity/offlinemessage.h"
#include <cstdint>
#include "../database/mysql/mysqlresult.h"
class OfflineMessageModel
{
    public:
    bool insert(OfflineMessage& message);

    std::vector<OfflineMessage> findByUserId(int userid);
    bool remove(std::int64_t id);
    bool clearUserMessages(int userid);
    int countPrivateUnread(int userId, int friendId);
    bool clearPrivateMessages(int userId, int friendId);
    bool markDelivered(std::int64_t id);
    std::vector<OfflineMessage> findPrivateMessages(int userId, int friendId);
    private:
    OfflineMessage makeOfflineMessage(const MysqlResult& result);
};