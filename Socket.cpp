/**
 * Project 66th
 */

#include "Socket.h"

/**
 * Socket implementation
 */
Socket::Socket()
{
    m_fd = socket(AF_INET,SOCK_STREAM,0);
    if(m_fd < 0){
        perror("socket create error");
        return;
    }

}

/**
 * @param fd
 */
Socket::Socket(int fd)
{
    m_fd = fd;
}

Socket::~Socket()
{
    close(m_fd);
}

/**
 * @return int
 */
int Socket::getFd() const
{
    return m_fd;
}
