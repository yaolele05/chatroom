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

    private:
    OfflineMessage makeOfflineMessage(const MysqlResult& result);
};