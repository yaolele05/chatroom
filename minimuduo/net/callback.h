#pragma once
#include <functional>
#include <memory>
class Buffer;
class TcpConnection;

using TcpConnectionptr=std::shared_ptr<TcpConnection>;
using Connectioncallback=std::function<void(const TcpConnectionptr&)>;
using Messagecallback=std::function<void(const TcpConnectionptr&, Buffer*)>;
using Closecallback=std::function<void(const TcpConnectionptr&)>;
using Writecomcallback=std::function<void(const TcpConnectionptr&)>;

