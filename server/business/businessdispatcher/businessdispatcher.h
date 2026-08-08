#pragma once 
#include <functional>
#include <unordered_map>
#include<string>
#include <nlohmann/json.hpp>
#include <mutex>
#include "../../../common/protocol/message.h"
using json=nlohmann::json;
class Session;

class BusinessDispatcher
{
    public:
    using Handler=std::function<void(const Message&,Session*)>;

    static BusinessDispatcher& instance();
    void registerHandler(const Messagetype type,Handler handler);
    bool dispatch(const Message& message,Session* session);

    private:
    BusinessDispatcher()=default;

     BusinessDispatcher(const BusinessDispatcher&)=delete;
     BusinessDispatcher& operator=(const BusinessDispatcher&)=delete;
    std::unordered_map<Messagetype,Handler>handlers_;

    std::mutex mutex_;
};