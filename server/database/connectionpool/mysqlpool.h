#pragma once
#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <string>
class MysqlClient;
class MysqlPool
{
    public:
    static MysqlPool& instance();
    
    
    /// 
    ///   
   

    public:
    bool init(const std::string& host,uint16_t port,const std::string& user,const std::string& password,const std::string& database,size_t  size );

    std::shared_ptr<MysqlClient> getConnection();
    void releaseConnection(std::shared_ptr<MysqlClient> conn);

    private:
    MysqlPool();
    ~MysqlPool();
    MysqlPool(const MysqlPool&)=delete;

    MysqlPool& operator=(const MysqlPool&)=delete;

    std::queue<std::shared_ptr<MysqlClient>> connections_;
    std::mutex mutex_;
    std::condition_variable cv_;
    int maxSize_{0};
    bool running_{false};
    
};