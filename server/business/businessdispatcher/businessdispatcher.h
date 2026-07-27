#pragma once 
#include <functional>
#include <unordered_map>
#include<string>
#include <nlohmann/json.hpp>

using json=nlohmann::json;
class Session;

class BusinessDispatcher
{
    public:
    using Handler=std::function<void(const json&,Session*)>;

    static BusinessDispatcher& instance();
    void registerHandler(const std::string& type,Handler handler);
    bool dispatch(const json& message,Session* session);

    private:
    BusinessDispatcher()=default;

    std::unordered_map<std::string,Handler>handlers_;

};