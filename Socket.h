/**
 * Project 66th
 */

#ifndef _SOCKET_H
#define _SOCKET_H
#include "NonCopyable.h"
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>

class Socket : public NonCopyable
{
public:
    Socket();

    /**
     * @param fd
     */
    explicit Socket(int fd); // 禁止隐式转换

    ~Socket();

    int getFd() const;

private:
    int m_fd;
};

#endif //_SOCKET_H
