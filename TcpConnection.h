/**
 * Project 66th
 */

#ifndef _TCPCONNECTION_H
#define _TCPCONNECTION_H
#include "SocketIO.h"
#include "Socket.h"
#include "InetAddress.h"

#include <functional>
#include <memory>
#include <sstream>
#include <string>

class EventLoop;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TcpConnectionCallback = std::function<void(const TcpConnectionPtr &)>;

class TcpConnection : public NonCopyable, public std::enable_shared_from_this<TcpConnection>
{
public:
    /**
     * @param fd
     */
    TcpConnection(int fd, EventLoop *loop);

    ~TcpConnection();

    bool isClosed();

    std::string receive();

    /**
     * @param msg
     */
    void send(const std::string &msg);

    void sendInLoop(const std::string &msg);

    std::string toString() const;
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

    void handleNewConnectionCallback();

    void handleMessageCallback();

    void handleCloseCallback();

private:
    Socket m_sock;
    SocketIO m_sockIO;
    InetAddress m_localAddr;
    InetAddress m_peerAddr;
    EventLoop *m_loop;

    InetAddress getLocalAddr();

    InetAddress getPeerAddr();

    TcpConnectionCallback m_onNewConnection;
    TcpConnectionCallback m_onMessage;
    TcpConnectionCallback m_onClose;
};

#endif //_TCPCONNECTION_H
