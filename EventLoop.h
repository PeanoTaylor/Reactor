/**
 * Project 66th
 */

#ifndef _EVENTLOOP_H
#define _EVENTLOOP_H
#include "Acceptor.h"
#include "TcpConnection.h"

#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <sys/epoll.h>
#include <thread>
#include <vector>

class EventLoop
{
public:
    using Functor = std::function<void()>;
    using AcceptCallback = std::function<void(int)>;

    /**
     * @param acc
     */
    explicit EventLoop(Acceptor *acc = nullptr);

    ~EventLoop();

    void loop();

    void unloop();

    /**
     * @param cb
     */
    void setNewConnectionCallback(const TcpConnectionCallback &cb);

    /**
     * @param cb
     */
    void setMessageCallback(const TcpConnectionCallback &cb);

    /**
     * @param cb
     */
    void setCloseCallback(const TcpConnectionCallback &cb);

    void setAcceptCallback(const AcceptCallback &cb);

    void addConnection(int connfd);

    void runInLoop(Functor &&cb);

    void queueInLoop(Functor &&cb);

private:
    static constexpr int kMaxEvents = 1024;

    int m_epfd;
    int m_eventfd;
    Acceptor *m_acceptor;
    std::vector<struct epoll_event> m_evlist;
    std::atomic<bool> m_isLooping;
    std::thread::id m_threadId;
    std::map<int, std::shared_ptr<TcpConnection>> m_conns;
    std::vector<Functor> m_pendingFunctors;
    std::mutex m_mutex;
    TcpConnectionCallback m_onNewConnection;
    TcpConnectionCallback m_onMessage;
    TcpConnectionCallback m_onClose;
    AcceptCallback m_onAccept;

    int createEpollFd();

    int createEventFd();

    /**
     * @param fd
     */
    void addEpollReadFd(int fd);

    /**
     * @param fd
     */
    void delEpollReadFd(int fd);

    void waitEpollFd();

    bool isInLoopThread() const;

    void wakeup();

    void handleRead();

    void doPendingFunctors();

    void handleNewConnection();

    void addConnectionInLoop(int connfd);

    /**
     * @param fd
     */
    void handleMessage(int fd);
};

#endif //_EVENTLOOP_H
