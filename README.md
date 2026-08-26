
# ChatRoom

## 软件要求

* Ubuntu 22.04+
* GCC 11+
* C++17
* CMake 3.16+
* Git
* MySQL 8.0+
* Redis 6.0+
* OpenSSL
* libcurl
* nlohmann/json

## 项目简介

ChatRoom 是一个基于 C++17 开发的客户端/服务器聊天室系统。

项目自己实现 MiniMuduo Reactor 网络库，基于 Linux `epoll` 实现网络事件驱动。

主要功能包括：

* 用户注册
* 密码登录
* 邮箱验证码登录
* 密码找回
* 好友管理
* 好友申请
* 好友屏蔽
* 私聊
* 群聊
* 聊天历史记录
* 离线/未读消息
* 心跳检测
* 文件传输
* 文件断点续传
* SHA-256 文件校验
* Base64 编解码
* MySQL 数据持久化
* Redis 缓存及状态管理
* 邮件验证码

## 项目架构

项目主要由以下模块组成：

```text
ChatRoom
├── client              # 客户端
├── server              # 服务端
├── common              # 公共模块
└── minimuduo           # Reactor 网络库
```

### MiniMuduo 网络层

```text
TcpServer
    │
    ├── Acceptor
    │
    └── EventLoop
            │
            ├── Channel
            ├── EpollPoller
            └── EventLoopThreadPool
                    │
                    └── TcpConnection
```

### 协议层

```text
Message
    ↓
JsonCodec
    ↓
JSON
    ↓
PacketCodec
    ↓
TCP
```

协议相关代码位于：

```text
common/protocol/
```

其中：

* `message`：消息对象
* `messagetype`：消息类型
* `JsonCodec`：JSON 编解码
* `PacketCodec`：TCP 数据包封装与拆包
* `protocol`：协议相关定义

### 服务端结构

```text
Client
   │
   │ TCP / SSL
   ↓
TcpServer
   ↓
Session
   ↓
BusinessDispatcher
   ↓
Service
   ├── LoginService
   ├── FriendService
   ├── GroupService
   ├── ChatService
   ├── HistoryService
   ├── OfflineService
   ├── FileService
   ├── EmailService
   └── HeartbeatService
        │
        ├── MySQL
        └── Redis
```

---

# 安装环境

## 安装编译工具
总：
```bash
sudo apt update

sudo apt install -y \
    build-essential \
    gcc \
    g++ \
    cmake \
    git \
 、
```
```
`sudo apt update
sudo apt install build-essential cmake
sudo apt install libssl-dev libcurl4-openssl-dev  libmysqlclient-dev libhiredis-dev
sudo apt install nlohmann-json3-dev`
```

检查 GCC：

```bash
gcc --version
g++ --version
```

检查 CMake：

```bash
cmake --version
```

项目使用 C++17 编译。

---

## 安装 OpenSSL

ChatRoom 使用 OpenSSL 实现：

* SHA-256

安装：

```bash
sudo apt install -y \
    openssl \
    libssl-dev
```

检查：

```bash
openssl version
```

---

## 安装 MySQL

ChatRoom 使用 MySQL 保存：

* 用户信息
* 好友关系
* 好友申请
* 群组信息
* 群成员信息
* 聊天消息
* 离线/未读消息
* 文件传输信息

安装：

```bash
sudo apt install -y \
    mysql-server \
    libmysqlclient-dev
```

启动 MySQL：

```bash
sudo systemctl enable mysql
sudo systemctl start mysql
```

检查 MySQL：

```bash
sudo systemctl status mysql
```

登录 MySQL：

```bash
mysql -u root -p
```

如果 Ubuntu 使用系统 root 用户认证，也可以：

```bash
sudo mysql
```

---

# 配置 MySQL

项目数据库结构应以项目中的 SQL 文件为准：

```text
sql/chatroom.sql
```

创建数据库：

```bash
mysql -u root -p < sql/chatroom.sql
```

默认数据库配置：

```text
数据库地址：127.0.0.1
数据库端口：3306
数据库名称：chatroom
数据库用户：root
数据库密码：123456
```

如果实际环境中的 MySQL 用户名、密码或地址不同，需要修改项目中的 MySQL 配置。

登录数据库：

```bash
mysql -u root -p
```

检查数据库：

```sql
SHOW DATABASES;
```

进入 ChatRoom 数据库：

```sql
USE chatroom;
```

查看数据表：

```sql
SHOW TABLES;
```

---

# 安装 Redis

ChatRoom 使用 Redis 保存实时状态和临时数据，包括：

* 用户在线状态
* 好友屏蔽关系
* 邮箱验证码
* 登录验证码
* 其他缓存数据

安装：

```bash
sudo apt install -y redis-server
```

启动 Redis：

```bash
sudo systemctl enable redis-server
sudo systemctl start redis-server
```

检查 Redis：

```bash
sudo systemctl status redis-server
```

测试 Redis：

```bash
redis-cli ping
```

正常情况下返回：

```text
PONG
```

默认 Redis 配置：

```text
Redis 地址：127.0.0.1
Redis 端口：6379
```

---

# 安装 libcurl

ChatRoom 的邮箱验证码功能使用 libcurl 连接 SMTP 服务器。

安装：

```bash
sudo apt install -y \
    curl \
    libcurl4-openssl-dev
```

检查：

```bash
curl --version
```

---

# 安装 nlohmann/json

ChatRoom 应用层协议使用 JSON。

安装：

```bash
sudo apt install -y nlohmann-json3-dev
```

安装完成后通常可以找到：

```text
/usr/include/nlohmann/json.hpp
```

项目中的 JSON 编解码主要位于：

```text
common/protocol/Jsoncodec.cpp
common/protocol/Jsoncodec.h
```

---

# 邮件配置

如果需要使用以下功能：

* 注册验证码
* 邮箱验证码登录
* 密码找回

需要配置 SMTP 邮箱。

项目可以使用 QQ 邮箱 SMTP。

配置示例：

```text
SMTP Server: smtp.qq.com
SMTP Port: 465
Email: 你的QQ邮箱@qq.com
Auth Code: QQ邮箱授权码
```

如果项目提供：

```text
config/email.conf.example
```

可以复制：

```bash
cp config/email.conf.example config/email.conf
```

然后修改：

```text
config/email.conf
```

例如：

```text
smtp_server=smtp.qq.com
smtp_port=465
email=你的邮箱@qq.com
auth_code=你的邮箱授权码
```

其中：

```text
email
```

填写自己的 QQ 邮箱。

```text
auth_code
```

填写 QQ 邮箱生成的 SMTP 授权码，而不是 QQ 邮箱登录密码。

---


# 项目编译

要
`sudo apt update
sudo apt install build-essential cmake
sudo apt install libssl-dev libcurl4-openssl-dev  libmysqlclient-dev libhiredis-dev
sudo apt install nlohmann-json3-dev`

## 进入项目根目录：

```bash
cd ~/chatroom
```

```
mkdir build
cd build
cmake ..
```

## 以后编译：

cd ~/chatroom/build
cmake ..
make -j

```

编译成功后：

```text
build/
├── client/
│   └── chatclient
├── server/
│   └── chatserver
├── common/
│   └── libcommon.a
└── minimuduo/
    └── libminimuduo.a
```

---

# 启动服务器

进入项目根目录：

```bash
cd ~/chatroom/build
```

启动服务器：
## 在build目录下
```bash
./server/chatserver
```

---

# 指定服务器 IP 和端口

服务器支持指定监听地址和端口：


# 启动客户端


启动方式
##在build目录下
例如：

./chatserver

默认：

0.0.0.0:8888

指定端口：

./server/chatserver --port 9999

指定 IP：

./server/chatserver --ip 127.0.0.1

两个一起：

./server/chatserver --ip 0.0.0.0 --port 9999
```bash
---

# 本机测试

首先启动 MySQL：

```bash
sudo systemctl start mysql
```

启动 Redis：

```bash
sudo systemctl start redis-server
```

然后启动服务器：

```bash
./server/chatserver
```

再打开一个终端：

./build/client/chatclient
```

如果需要测试两个客户端，可以打开两个终端：

终端 1：

```bash
./build/client/chatclient
```

终端 2：

```bash
./build/client/chatclient
```

---


# 局域网测试

如果需要让其他电脑运行客户端，需要让服务器监听局域网地址。

服务器：

```bash
./server/chatserver 0.0.0.0 8888
```

查看服务器 IP：

```bash
ip addr
```
其他电脑运行：

```bash
在build下
./client/chatclient --ip 192.168.1.100  --port 8888
```

如果无法连接，需要检查防火墙：

```bash
sudo ufw status
```

如果启用了 UFW，可以开放服务器端口：

```bash
sudo ufw allow 8888/tcp
```

---
