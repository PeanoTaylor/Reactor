/**
 * Project 66th
 */

#include "InetAddress.h"

/**
 * InetAddress implementation
 */

/**
 * @param ip
 * @param port
 */
InetAddress::InetAddress(const std::string &ip, unsigned short port)
{
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
}

/**
 * @param addr
 */
InetAddress::InetAddress(const struct sockaddr_in &addr)
{
    m_addr = addr;
}

InetAddress::~InetAddress()
{
}

/**
 * @return string
 */
std::string InetAddress::getIp() const
{
    return std::string(inet_ntoa(m_addr.sin_addr)); // 网络字节序转为x.x.x.x
}

/**
 * @return unsigned short
 */
unsigned short InetAddress::getPort() const
{
    return ntohs(m_addr.sin_port); // 网络字节序转为主机字节序
}

/**
 * @return const struct sockaddr_in *
 */
const struct sockaddr_in *InetAddress::getInetAddressPtr() const
{
    return &m_addr;
}
