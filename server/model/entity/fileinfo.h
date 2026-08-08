#pragma once

#include <string>
#include <chrono>
#include <cstdint>

class FileInfo
{
public:

    std::int64_t id() const
    {
        return id_;
    }
    void setId(std::int64_t id)
    {
        id_ = id;
    }
    int senderId() const
    {
        return senderId_;
    }
    void setSenderId(int id)
    {
        senderId_ = id;
    }
    int receiverId() const
    {
        return receiverId_;
    }
    void setReceiverId(std::int64_t id)
    {
        receiverId_ = id;
    }
    std::int64_t groupId() const
    {
        return groupId_;
    }
    void setGroupId(int id)
    {
        groupId_ = id;
    }
    const std::string& fileName() const
    {
        return fileName_;
    }
    void setFileName(const std::string& name)
    {
        fileName_ = name;
    }
    const std::string& filePath() const
    {
        return filePath_;
    }
    void setFilePath(const std::string& path)
    {
        filePath_ = path;
    }
    std::uint64_t fileSize() const
    {
        return fileSize_;
    }
    void setFileSize(std::uint64_t size)
    {
        fileSize_ = size;
    }
    const std::string& fileSha256() const
    {
        return fileSha256_;
    }

    void setFileSha256(const std::string& sha256)
    {
        fileSha256_= sha256;
    }
    std::uint64_t transferredSize() const
    {
        return transferredSize_;
    }
    void setTransferredSize(std::uint64_t size)
    {
        transferredSize_ = size;
    }
    bool completed() const
    {
        return completed_;
    }
    void setCompleted(bool completed)
    {
        completed_ = completed;
    }


    std::chrono::system_clock::time_point createTime() const
    {
        return createTime_;
    }

    void setCreateTime(
        std::chrono::system_clock::time_point time)
    {
        createTime_ = time;
    }


private:

    std::int64_t id_{0};
    int senderId_{0};
    int receiverId_{0};
    std::int64_t  groupId_{0};

    std::string fileName_;
    // 服务端保存路径
    std::string filePath_;
    std::uint64_t fileSize_{0};
    // 秒传和完整性校验
    std::string fileSha256_;
    std::uint64_t transferredSize_{0};
    bool completed_{false};
    std::chrono::system_clock::time_point createTime_;
};