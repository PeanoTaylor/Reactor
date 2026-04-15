/**
 * Project 66th
 */

#include "Acceptor.h"

/**
 * Acceptor implementation
 */

/**
 * @param ip
 * @param port
 */
Acceptor::Acceptor(const std::string &ip, unsigned short port) : m_sock(), m_addr(ip, port)
{
}

Acceptor::~Acceptor()
{
}

/**
 * @return void
 */
void Acceptor::ready()
{
    setReuseAddr();
    setReusePort();
    bind();
    listen();
}

/**
 * @return int
 */
int Acceptor::accept()
{
    int connfd = ::accept(m_sock.getFd(), nullptr, nullptr);
    if (connfd < 0)
    {
        perror("accept failure!");
    }
    return connfd;
}

/**
 * @return void
 */
void Acceptor::setReuseAddr()
{
    // 设置地址复用
    int opt = 1;
    setsockopt(m_sock.getFd(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

/**
 * @return void
 */
void Acceptor::setReusePort()
{
    // 设置端口复用
    int opt = 1;
    setsockopt(m_sock.getFd(), SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
    return;
}

/**
 * @return void
 */
void Acceptor::bind()
{
    int ret = ::bind(
        m_sock.getFd(),
        reinterpret_cast<const struct sockaddr *>(m_addr.getInetAddressPtr()),
        sizeof(struct sockaddr_in));
    if (ret < 0)
    {
        perror("bind failure!");
        exit(EXIT_FAILURE);
    }
}

/**
 * @return void
 */
void Acceptor::listen()
{
    int ret = ::listen(m_sock.getFd(), 128);
    if (ret < 0)
    {
        perror("listen failure!");
        exit(EXIT_FAILURE);
    }
}
