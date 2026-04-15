/**
 * Project 66th
 */

#include "TcpServer.h"

#include "MyTask.h"

#include <functional>
#include <iostream>
#include <string>
#include <unistd.h>

TcpServer::TcpServer(const std::string &ip,
                     unsigned short port,
                     size_t thread_num,
                     size_t queue_size,
                     size_t sub_reactor_num)
    : m_acceptor(ip, port),
      m_mainLoop(&m_acceptor),
      m_nextReactor(0),
      m_threadPool(thread_num, queue_size)
{
    using std::placeholders::_1;

    // sub-reactor 数量至少为 1，避免没有 IO reactor 可分发连接。
    const size_t reactor_num = sub_reactor_num == 0 ? 1 : sub_reactor_num;
    m_subReactors.reserve(reactor_num);
    for (size_t idx = 0; idx < reactor_num; ++idx)
    {
        m_subReactors.emplace_back(new EventLoop());
        m_subReactors.back()->setNewConnectionCallback(
            std::bind(&TcpServer::handleNewConnection, this, _1));
        m_subReactors.back()->setMessageCallback(
            std::bind(&TcpServer::handleMessage, this, _1));
        m_subReactors.back()->setCloseCallback(
            std::bind(&TcpServer::handleClose, this, _1));
    }

    // main-reactor 只负责 accept，不直接处理连接 IO。
    // 新连接统一交给 TcpServer 做负载分发（round-robin 到 sub-reactor）。
    m_mainLoop.setAcceptCallback(
        std::bind(&TcpServer::dispatchConnection, this, _1));
}

TcpServer::~TcpServer()
{
    m_mainLoop.unloop();
    stopSubReactors();
    m_threadPool.stop();
}

void TcpServer::start()
{
    m_acceptor.ready();
    m_threadPool.start();
    startSubReactors();
    m_mainLoop.loop();
    stopSubReactors();
    m_threadPool.stop();
}

// 每个 sub-reactor 对应一个独立线程，专门处理分配给它的连接读写事件。
// 这样 main-reactor 不会因连接 IO 被阻塞。
void TcpServer::startSubReactors()
{
    m_subThreads.reserve(m_subReactors.size());
    for (auto &reactor : m_subReactors)
    {
        EventLoop *sub_reactor = reactor.get();
        m_subThreads.emplace_back([sub_reactor]()
                                  { sub_reactor->loop(); });
    }
}

void TcpServer::stopSubReactors()
{
    for (auto &reactor : m_subReactors)
    {
        reactor->unloop();
    }

    for (auto &thread : m_subThreads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }
    m_subThreads.clear();
}

// 轮询分发策略：将新连接均匀分布到不同 sub-reactor，
// 实现简单、开销低，适合当前版本。
EventLoop *TcpServer::getNextSubReactor()
{
    if (m_subReactors.empty())
    {
        return nullptr;
    }
    EventLoop *reactor = m_subReactors[m_nextReactor].get();
    m_nextReactor = (m_nextReactor + 1) % m_subReactors.size();
    return reactor;
}

void TcpServer::dispatchConnection(int connfd)
{
    EventLoop *sub_reactor = getNextSubReactor();
    if (sub_reactor == nullptr)
    {
        close(connfd);
        return;
    }
    // accept 得到的 connfd 交给目标 sub-reactor 接管。
    // 后续该连接的可读/可写事件只在目标 sub-reactor 所在线程处理。
    sub_reactor->addConnection(connfd);
}

void TcpServer::handleNewConnection(const TcpConnectionPtr &conn)
{
    std::cout << "new client: " << conn->toString() << std::endl;
}

void TcpServer::handleMessage(const TcpConnectionPtr &conn)
{
    const std::string msg = conn->receive();
    std::cout << "recv from client: " << msg;

    MyTask task(msg, conn);
    m_threadPool.addTask([task]() mutable
                         { task.process(); });
}

void TcpServer::handleClose(const TcpConnectionPtr &conn)
{
    std::cout << "client closed: " << conn->toString() << std::endl;
}
