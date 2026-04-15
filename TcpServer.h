/**
 * Project 66th
 */

#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#include "Acceptor.h"
#include "EventLoop.h"
#include "ThreadPool.h"

#include <cstddef>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class TcpServer
{
public:
    TcpServer(const std::string &ip,
              unsigned short port,
              size_t thread_num,
              size_t queue_size,
              size_t sub_reactor_num);

    ~TcpServer();

    void start();

private:
    void startSubReactors();

    void stopSubReactors();

    EventLoop *getNextSubReactor();

    void dispatchConnection(int connfd);

    void handleNewConnection(const TcpConnectionPtr &conn);

    void handleMessage(const TcpConnectionPtr &conn);

    void handleClose(const TcpConnectionPtr &conn);

    Acceptor m_acceptor;
    EventLoop m_mainLoop;
    size_t m_nextReactor;
    std::vector<std::unique_ptr<EventLoop>> m_subReactors;
    std::vector<std::thread> m_subThreads;
    ThreadPool m_threadPool;
};

#endif //_TCPSERVER_H
