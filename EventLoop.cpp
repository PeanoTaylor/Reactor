/**
 * Project 66th
 */

#include "EventLoop.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sys/eventfd.h>
#include <unistd.h>

/**
 * EventLoop implementation
 */

/**
 * @param acc
 */
EventLoop::EventLoop(Acceptor *acc)
    : m_epfd(createEpollFd()),
      m_eventfd(createEventFd()),
      m_acceptor(acc),
      m_evlist(kMaxEvents),
      m_isLooping(false),
      m_threadId(std::this_thread::get_id())
{
    if (m_acceptor != nullptr)
    {
        addEpollReadFd(m_acceptor->getFd());
    }
    addEpollReadFd(m_eventfd);
}

EventLoop::~EventLoop()
{
    if (m_eventfd >= 0)
    {
        close(m_eventfd);
    }
    if (m_epfd >= 0)
    {
        close(m_epfd);
    }
}

/**
 * @return void
 */
void EventLoop::loop()
{
    m_threadId = std::this_thread::get_id();
    m_isLooping = true;
    while (m_isLooping)
    {
        waitEpollFd();
    }
}

/**
 * @return void
 */
void EventLoop::unloop()
{
    m_isLooping = false;
    wakeup();
}

void EventLoop::setNewConnectionCallback(const TcpConnectionCallback &cb)
{
    m_onNewConnection = cb;
}

void EventLoop::setMessageCallback(const TcpConnectionCallback &cb)
{
    m_onMessage = cb;
}

void EventLoop::setCloseCallback(const TcpConnectionCallback &cb)
{
    m_onClose = cb;
}

void EventLoop::setAcceptCallback(const AcceptCallback &cb)
{
    m_onAccept = cb;
}

void EventLoop::addConnection(int connfd)
{
    // 跨线程接管连接入口：确保连接注册和 TcpConnection 创建
    // 在目标 sub-reactor 的 loop 线程内执行，避免并发竞态。
    runInLoop([this, connfd]()
              { addConnectionInLoop(connfd); });
}

/**
 * 在EventLoop线程中执行一个任务（核心入口函数）
 * 1. 如果当前线程就是EventLoop所在的IO线程，直接执行回调
 * 2. 如果不是（比如是工作线程），就将任务放入队列，并唤醒EventLoop
 * @param cb 要执行的任务（函数/lamdba）
 */
void EventLoop::runInLoop(Functor &&cb)
{
    // 判断：当前调用线程 是否是 EventLoop 所属的IO线程
    if (isInLoopThread())
    {
        // 如果在IO线程中，直接执行任务（最常见：send数据、关闭连接等）
        cb();
    }
    else
    {
        // 如果不在IO线程（比如线程池的工作线程）
        // 将任务放入队列，让IO线程异步执行
        queueInLoop(std::move(cb)); // 业务线程
    }
}

/**
 * 将任务加入EventLoop的待执行队列，并唤醒IO线程
 * 作用：跨线程让IO线程执行指定任务
 * @param cb 要异步执行的任务
 */
void EventLoop::queueInLoop(Functor &&cb)
{
    // 加锁：多线程同时操作任务队列，必须保证线程安全
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        // 将任务（移动语义，高性能）加入pending队列
        m_pendingFunctors.push_back(std::move(cb));
    } // 离开作用域，自动解锁

    // 唤醒处于epoll_wait中的EventLoop线程
    // 让它立即去执行队列中的任务，而不是继续等待
    wakeup();
}

/**
 * @return int
 */
int EventLoop::createEpollFd()
{
    const int epfd = epoll_create1(0);
    if (epfd < 0)
    {
        perror("epoll create failure!");
        exit(EXIT_FAILURE);
    }
    return epfd;
}

int EventLoop::createEventFd()
{
    const int eventfd = ::eventfd(0, 0);
    if (eventfd < 0)
    {
        perror("eventfd create failure!");
        exit(EXIT_FAILURE);
    }
    return eventfd;
}

/**
 * @param fd
 * @return void
 */
void EventLoop::addEpollReadFd(int fd)
{
    struct epoll_event ev = {};

    ev.events = EPOLLIN; // 监听 读事件
    ev.data.fd = fd;
    int ret = epoll_ctl(m_epfd, EPOLL_CTL_ADD, fd, &ev);
    if (ret < 0)
    {
        perror("epoll add failure!");
        exit(EXIT_FAILURE);
    }
}

/**
 * @param fd
 * @return void
 */
void EventLoop::delEpollReadFd(int fd)
{
    int ret = epoll_ctl(m_epfd, EPOLL_CTL_DEL, fd, nullptr);
    if (ret < 0)
    {
        perror("epoll del failure!");
        exit(EXIT_FAILURE);
    }
}

/**
 * @return void
 */
void EventLoop::waitEpollFd()
{
    const int nready = epoll_wait(m_epfd, m_evlist.data(), m_evlist.size(), -1);
    if (nready < 0)
    {
        perror("epoll wait failure!");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < nready; ++i)
    {
        if ((m_evlist[i].events & EPOLLIN) == 0)
        {
            continue;
        }
        // 带 acceptor 的 EventLoop 是 main-reactor：处理监听 fd。
        // 不带 acceptor 的 EventLoop 是 sub-reactor：只处理连接 fd 和 eventfd。
        if (m_acceptor != nullptr && m_evlist[i].data.fd == m_acceptor->getFd())
        {
            handleNewConnection();
        }
        else if (m_evlist[i].data.fd == m_eventfd)
        {
            handleRead();
            doPendingFunctors();
        }
        else
        {
            handleMessage(m_evlist[i].data.fd);
        }
    }
}

bool EventLoop::isInLoopThread() const
{
    return m_threadId == std::this_thread::get_id();
}

void EventLoop::wakeup()
{
    const uint64_t one = 1;
    const ssize_t nwrite = write(m_eventfd, &one, sizeof(one));
    if (nwrite != static_cast<ssize_t>(sizeof(one)))
    {
        perror("eventfd write failure!");
    }
}

void EventLoop::handleRead()
{
    uint64_t one = 0;
    const ssize_t nread = read(m_eventfd, &one, sizeof(one));
    if (nread != static_cast<ssize_t>(sizeof(one)))
    {
        perror("eventfd read failure!");
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        functors.swap(m_pendingFunctors);
    }

    for (auto &functor : functors)
    {
        functor();
    }
}

/**
 * @return void
 */
void EventLoop::handleNewConnection()
{
    if (m_acceptor == nullptr)
    {
        return;
    }

    const int connfd = m_acceptor->accept();
    if (connfd < 0)
    {
        perror("accept failure!");
        exit(EXIT_FAILURE);
    }

    // 若注册了 m_onAccept，说明当前是 main-reactor 模式：
    // accept 后不本地建连接，直接回调上层分发到 sub-reactor。
    if (m_onAccept)
    {
        m_onAccept(connfd);
        return;
    }
    addConnectionInLoop(connfd);
}

// 真正把 connfd 加入当前 reactor 的 epoll，并绑定连接级回调。
// 从这里开始，连接生命周期归当前 sub-reactor 管理。
void EventLoop::addConnectionInLoop(int connfd)
{
    addEpollReadFd(connfd);
    TcpConnectionPtr conn(new TcpConnection(connfd, this));
    conn->setNewConnectionCallback(m_onNewConnection);
    conn->setMessageCallback(m_onMessage);
    conn->setCloseCallback(m_onClose);
    m_conns[connfd] = conn;
    conn->handleNewConnectionCallback();
}

/**
 * @param fd
 * @return void
 */
void EventLoop::handleMessage(int fd)
{
    auto it = m_conns.find(fd);
    if (it != m_conns.end())
    {
        if (it->second->isClosed())
        {
            it->second->handleCloseCallback();
            delEpollReadFd(fd);
            m_conns.erase(it);
        }
        else
        {
            it->second->handleMessageCallback();
        }
    }
}
