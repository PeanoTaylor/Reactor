/**
 * Project 66th
 */

#ifndef _INETADDRESS_H
#define _INETADDRESS_H
#include <string>
#include <netinet/in.h>   // 核心：定义 sockaddr_in
#include <sys/socket.h>   // 配套 socket 函数
#include <arpa/inet.h>    // 配套 IP 转换函数（inet_pton等）
class InetAddress
{
public:
    /**
     * @param ip
     * @param port
     */
    InetAddress(const std::string &ip, unsigned short port);

    /**
     * @param addr
     */
    InetAddress(const struct sockaddr_in &addr);

    ~InetAddress();

    std::string getIp() const;

    unsigned short getPort() const;

    const struct sockaddr_in *getInetAddressPtr() const;

private:
    struct sockaddr_in m_addr;
};

#endif //_INETADDRESS_H
