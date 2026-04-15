/**
 * Project 66th
 */

#ifndef _ACCEPTOR_H
#define _ACCEPTOR_H
#include "Socket.h"
#include "InetAddress.h"
#include <string>

class Acceptor
{
public:
    /**
     * @param ip
     * @param port
     */
    Acceptor(const std::string &ip, unsigned short port);

    ~Acceptor();

    void ready();

    int accept();

    int getFd() const { return m_sock.getFd(); }

private:
    Socket m_sock;
    InetAddress m_addr;

    void setReuseAddr();

    void setReusePort();

    void bind();

    void listen();
};

#endif //_ACCEPTOR_H
