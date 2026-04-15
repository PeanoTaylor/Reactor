# Reactor

一个基于 `epoll` 的 C++ 网络服务示例，采用**主从 Reactor + 线程池业务处理**模型。

当前实现目标：

- `main-reactor` 负责监听与 `accept`
- `sub-reactor` 负责连接读写事件处理
- `ThreadPool` 负责异步业务任务执行
- `MyTask` 演示业务处理后回包

---

## 1. 架构概览

### 1.1 线程与职责

- 主线程（`main-reactor`）：
  - 监听服务端口
  - 接收新连接
  - 按 round-robin 分发给各 `sub-reactor`
- `sub-reactor` 线程组：
  - 每个线程拥有一个 `EventLoop`
  - 管理分配到本线程的连接 fd
  - 触发连接、消息、关闭回调
- 业务线程池：
  - 处理 `MyTask` 等业务逻辑
  - 避免业务阻塞 IO 线程

### 1.2 主流程

1. `TcpServer::start()` 启动监听、线程池与 sub-reactor 线程。
2. `main-reactor` 接收连接并分发给某个 sub-reactor。
3. sub-reactor 在连接可读时触发消息回调。
4. 消息回调构造 `MyTask` 并提交到线程池。
5. 业务处理完成后通过连接对象发送响应。

---

## 2. 目录与模块

- `Acceptor.*`：监听 socket 封装（`bind/listen/accept`）
- `EventLoop.*`：`epoll` 事件循环与连接管理
- `TcpConnection.*`：连接收发、连接描述、连接回调
- `TcpServer.*`：主从 Reactor 组装与生命周期管理
- `TaskQueue.*`：线程池任务队列（生产者-消费者）
- `ThreadPool.*`：工作线程管理与任务执行
- `MyTask.h`：示例业务任务
- `server.cpp`：服务端入口
- `client.cpp`：简易客户端测试程序

---

## 3. 关键类职责

### 3.1 `TcpServer`

核心职责：

- 创建并持有 `Acceptor`、`main-reactor`、`sub-reactor` 组、`ThreadPool`
- 注册连接/消息/关闭回调
- 负责连接分发策略（轮询）

关键方法：

- `start()`
- `dispatchConnection(int connfd)`
- `getNextSubReactor()`

### 3.2 `EventLoop`

核心职责：

- 封装 `epoll_wait` 事件循环
- 区分监听 fd、唤醒 fd、连接 fd
- 管理 `TcpConnection` 生命周期
- 提供跨线程任务投递（`runInLoop/queueInLoop`）

### 3.3 `ThreadPool` + `TaskQueue`

核心职责：

- 将业务任务与 IO 线程解耦
- 保证任务并发执行与安全退出

### 3.4 `MyTask`

演示业务处理逻辑：

- 输入：客户端消息 + 连接对象
- 处理：简单拼接 `"msg: " + 原消息`
- 输出：发送响应给客户端

---

## 4. 编译

在 `Project` 上级目录执行：

```bash
g++ -std=c++11 -Wall -Wextra -pedantic Acceptor.cpp EventLoop.cpp InetAddress.cpp Socket.cpp SocketIO.cpp TcpConnection.cpp TaskQueue.cpp ThreadPool.cpp TcpServer.cpp server.cpp -o reactor_server
```

```bash
g++ -std=c++11 -Wall -Wextra -pedantic client.cpp -o reactor_client
```

---

## 5. 运行

先启动服务端：

```bash
./reactor_server
```

再启动客户端：

```bash
./reactor_client
```

客户端输入任意文本并回车，可看到服务端日志与客户端响应。

---

## 6. 当前可继续优化方向

- I/O 模型优化：`accept4 + 非阻塞 + ET + RDHUP + 读到 EAGAIN`
- 连接容器优化：`std::map` -> `std::unordered_map`
- 任务对象优化：减少 `std::function` 高频分配开销
- 日志优化：高并发场景改异步日志或采样日志

---

## 7. 备注

本项目用于学习 Reactor 模式与线程池协作实现，代码结构已具备扩展为小型网络库的基础。
