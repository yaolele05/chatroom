#pragma once
#include <cstdint>
#include <chrono>
class FileReceiver
{
public:

    enum Status
    {
        Waiting = 0,     //等待下载
        Downloading = 1, //下载中
        Finished = 2     //完成
    };


public:
    FileReceiver():id_(0),fileId_(0),userId_(0),status_(Waiting)
    {

    }

    int64_t id() const
    {
        return id_;
    }
    void setId(int64_t id)
    {
        id_=id;
    }
    int64_t fileId() const
    {
        return fileId_;
    }
    void setFileId(int64_t id)
    {
        fileId_=id;
    }
    int userId() const
    {
        return userId_;
    }
    void setUserId(int id)
    {
        userId_=id;
    }
    int status() const
    {
        return status_;
    }
    void setStatus(int status)
    {
        status_=status;
    }
    std::chrono::system_clock::time_point createTime() const
    {
        return createTime_;
    }
    void setCreateTime(std::chrono::system_clock::time_point time)
    {
        createTime_=time;
    }



private:
    int64_t id_;
    int64_t fileId_;
    int userId_;
    int status_;
    std::chrono::system_clock::time_point createTime_;

};