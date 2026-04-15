/**
 * Project 66th
 */

#ifndef _SOCKETIO_H
#define _SOCKETIO_H
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <iostream> 
class SocketIO
{
public:
    /**
     * @param fd
     */
    explicit SocketIO(int fd);

    ~SocketIO();

    /**
     * @param buf
     * @param len
     */
    int readn(char *buf, int len);

    /**
     * @param buf
     * @param len
     */
    int readLine(char *buf, int len);

    /**
     * @param buf
     * @param len
     */
    int writen(const char *buf, int len);

private:
    int m_fd;
};

#endif //_SOCKETIO_H
