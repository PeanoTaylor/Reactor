/**
 * Project 66th
 */

#include "TcpConnection.h"

#include "EventLoop.h"

#include <cerrno>
#include <cstdio>

/**
 * TcpConnection implementation
 */

/**
 * @param fd
 */
TcpConnection::TcpConnection(int fd, EventLoop *loop)
    : m_sock(fd),
      m_sockIO(fd),
      m_localAddr(getLocalAddr()),
      m_peerAddr(getPeerAddr()),
      m_loop(loop)
{
}

TcpConnection::~TcpConnection()
{
}

bool TcpConnection::isClosed()
{
    char buf;
    const int ret = recv(m_sock.getFd(), &buf, sizeof(buf), MSG_PEEK);
    return ret == 0;
}

/**
 * @return string
 */
std::string TcpConnection::receive()
{
    char buf[65535] = {0};
    int ret = m_sockIO.readLine(buf, sizeof(buf));
    if (ret > 0)
    {
        return std::string(buf);
    }
    else
    {
        // 读到0表示对端关闭，<0表示出错
        return std::string();
    }
}

/**
 * @param msg
 * @return void
 */
void TcpConnection::send(const std::string &msg)
{
    m_sockIO.writen(msg.c_str(), msg.size());
}

void TcpConnection::sendInLoop(const std::string &msg)
{
    TcpConnectionPtr self = shared_from_this();
    m_loop->runInLoop([self, msg]() { self->send(msg); });
}

/**
 * @return string
 */
std::string TcpConnection::toString() const
{
    std::ostringstream oss;
    oss << m_localAddr.getIp() << ":" << m_localAddr.getPort() << "->" << m_peerAddr.getIp() << ":" << m_peerAddr.getPort();
    return oss.str();
}

/**
 * @return InetAddress
 */
InetAddress TcpConnection::getLocalAddr()
{
    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    int ret = getsockname(m_sock.getFd(), (struct sockaddr *)&localaddr, &addrlen);
    if (ret < 0)
    {
        perror("get local addr failure!");
    }
    return InetAddress(localaddr);
}

/**
 * @return InetAddress
 */
InetAddress TcpConnection::getPeerAddr()
{
    struct sockaddr_in peeraddr;
    socklen_t addrlen = sizeof(peeraddr);
    int ret = getpeername(m_sock.getFd(), (struct sockaddr *)&peeraddr, &addrlen);
    if (ret < 0)
    {
        perror("get peer addr failure!");
    }
    return InetAddress(peeraddr);
}

/**
 * @param cb
 */
void TcpConnection::setNewConnectionCallback(const TcpConnectionCallback &cb)
{
    m_onNewConnection = cb;
}

/**
 * @param cb
 */
void TcpConnection::setMessageCallback(const TcpConnectionCallback &cb)
{
    m_onMessage = cb;
}

/**
 * @param cb
 */
void TcpConnection::setCloseCallback(const TcpConnectionCallback &cb)
{
    m_onClose = cb;
}

/**
 * @return void
 */
void TcpConnection::handleNewConnectionCallback()
{
    if (m_onNewConnection)
    {
        m_onNewConnection(shared_from_this());
    }
}

/**
 * @return void
 */
void TcpConnection::handleMessageCallback()
{
    if (m_onMessage)
    {
        m_onMessage(shared_from_this());
    }
}

/**
 * @return void
 */
void TcpConnection::handleCloseCallback()
{
    if (m_onClose)
    {
        m_onClose(shared_from_this());
    }
}
