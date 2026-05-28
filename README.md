第一阶段
├── Reactor框架
├── JSON协议
├── 登录注册
└── 在线私聊

第二阶段
├── Redis
├── 好友系统
├── 离线消息
└── 在线状态

第三阶段
├── 群聊
├── 群管理
├── 历史消息
└── 心跳机制

第四阶段
├── 文件发送
├── 图片压缩
└── 断点续传

第五阶段
├── TLS
├── 压测
├── 日志系统
└── 文档

ChatRoom/
│
├── CMakeLists.txt
├── README.md
│
├── build/
├── bin/
├── logs/
├── docs/
│
├── third_party/
│   │
│   ├── json/
│   ├── hiredis/
│   ├── openssl/
│   ├── spdlog/
│   └── opencv/
│
├── protocol/
│   │
│   ├── message.h
│   ├── message.cpp
│   │
│   ├── msgtype.h
│   └── codec.h
│
├── common/
│   │
│   ├── util/
│   │   ├── singleton.h
│   │   ├── timestamp.h
│   │   ├── uuid.h
│   │   └── stringutil.h
│   │
│   ├── config/
│   │   ├── config.h
│   │   └── config.cpp
│   │
│   ├── logger/
│   │   ├── logger.h
│   │   └── logger.cpp
│   │
│   └── crypto/
│       ├── sha256.h
│       ├── sha256.cpp
│       ├── aes.h
│       └── aes.cpp
│
├── network/
│   │
│   ├── reactor/
│   │   │
│   │   ├── epoll.h
│   │   ├── epoll.cpp
│   │   │
│   │   ├── channel.h
│   │   ├── channel.cpp
│   │   │
│   │   ├── eventloop.h
│   │   ├── eventloop.cpp
│   │   │
│   │   ├── timer.h
│   │   ├── timer.cpp
│   │   │
│   │   ├── timerqueue.h
│   │   └── timerqueue.cpp
│   │
│   ├── socket/
│   │   ├── socket.h
│   │   └── socket.cpp
│   │
│   ├── buffer/
│   │   ├── buffer.h
│   │   └── buffer.cpp
│   │
│   ├── connection/
│   │   ├── tcpconnection.h
│   │   ├── tcpconnection.cpp
│   │   │
│   │   ├── sslconnection.h
│   │   └── sslconnection.cpp
│   │
│   ├── acceptor/
│   │   ├── acceptor.h
│   │   └── acceptor.cpp
│   │
│   └── server/
│       ├── tcpserver.h
│       └── tcpserver.cpp
│
├── database/
│   │
│   ├── redis/
│   │   ├── redisclient.h
│   │   └── redisclient.cpp
│   │
│   └── repository/
│       ├── userrepo.h
│       ├── friendrepo.h
│       ├── grouprepo.h
│       ├── messagerepo.h
│       └── filerepo.h
│
├── business/
│   │
│   ├── user/
│   │   ├── userservice.h
│   │   └── userservice.cpp
│   │
│   ├── friend/
│   │   ├── friendservice.h
│   │   └── friendservice.cpp
│   │
│   ├── group/
│   │   ├── groupservice.h
│   │   └── groupservice.cpp
│   │
│   ├── chat/
│   │   ├── chatservice.h
│   │   └── chatservice.cpp
│   │
│   └── file/
│       ├── fileservice.h
│       └── fileservice.cpp
│
├── model/
│   │
│   ├── user.h
│   ├── user.cpp
│   │
│   ├── friend.h
│   ├── friend.cpp
│   │
│   ├── group.h
│   ├── group.cpp
│   │
│   ├── groupmember.h
│   ├── groupmember.cpp
│   │
│   ├── message.h
│   ├── message.cpp
│   │
│   ├── fileinfo.h
│   └── fileinfo.cpp
│
├── storage/
│   │
│   ├── images/
│   │
│   ├── files/
│   │
│   ├── avatar/
│   │
│   └── temp/
│
├── media/
│   │
│   ├── image/
│   │   ├── compressor.h
│   │   └── compressor.cpp
│   │
│   └── file/
│       ├── chunk.h
│       ├── chunk.cpp
│       │
│       ├── resumefile.h
│       └── resumefile.cpp
│
├── security/
│   │
│   ├── ssl/
│   │   ├── sslcontext.h
│   │   ├── sslcontext.cpp
│   │   │
│   │   ├── cert/
│   │   │   ├── server.crt
│   │   │   └── server.key
│   │   │
│   │   └── sslmanager.cpp
│   │
│   └── auth/
│       ├── token.h
│       └── token.cpp
│
├── client/
│   │
│   ├── cli/
│   │   ├── main.cpp
│   │   ├── command.cpp
│   │   └── command.h
│   │
│   ├── network/
│   │   ├── clientconnection.cpp
│   │   └── clientconnection.h
│   │
│   ├── chat/
│   │
│   ├── file/
│   │
│   └── image/
│
├── tests/
│   │
│   ├── test_redis.cpp
│   ├── test_login.cpp
│   ├── test_group.cpp
│   ├── test_file.cpp
│   └── test_tls.cpp
│
└── tools/
    │
    ├── pressure_test/
    │   ├── benchmark.cpp
    │   └── robot.cpp
    │
    └── scripts/





chatroom
|
├── net
│   ├── eventloop
│   ├── epoll
│   ├── channel
│   └── connection
│
├── protocol
│   ├── message.h
│   ├── message.cpp
│   ├── codec.h
│   └── codec.cpp
│
├── business
│   ├── usermanager
│   ├── friendmanager
│   └── chatmanager
│
├── server
│   └── chatserver
│
└── main.cpp
