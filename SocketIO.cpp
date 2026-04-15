/**
 * Project 66th
 */

#include "SocketIO.h"

#include <cstdio>

/**
 * SocketIO implementation
 */

/**
 * @param fd
 */
SocketIO::SocketIO(int fd):m_fd(fd)
{
}

SocketIO::~SocketIO()
{
}

/**
 * @param buf
 * @param len
 * @return int
 */
int SocketIO::readn(char *buf, int len)
{
    int left = len;
    char *pstr = buf;
    while (left > 0)
    {
        int nread = read(m_fd, pstr, left);
        if (nread == 0)
        {
            break;
        }
        else if (nread == -1 && errno == EINTR)
        {
            continue;
        }
        else if (nread == -1)
        {
            return -1;
        }
        else
        {
            left -= nread;
            pstr += nread;
        }
    }
    return len - left;
}

/**
 * @param buf
 * @param len
 * @return int
 */
int SocketIO::readLine(char *buf, int len)
{
    int total = 0, ret = 0;
    int left = len - 1; // 预留1字节放字符串结尾'\0'

    char *pstr = buf;
    while (left > 0)
    {
        ret = recv(m_fd, pstr, left, MSG_PEEK); // 尝试窥探缓冲区，非实际读出,避免拆包/粘包
        if (ret == 0)
        {
            break; // 对端关闭
        }
        else if (ret == -1 && errno == EINTR)
        {
            continue; // 对当前进程（线程）而言发生了中断//中断触发，导致CPU的使用权被抢夺，后续应该继续执行
        }
        else if (ret == -1)
        {
            perror("readline");
            return -1;
        }
        else // peek到数据，查找有没有 '\n'
        {
            int idx = 0;
            for (; idx < ret; ++idx)
            {
                if (pstr[idx] == '\n')
                {
                    int sz = idx + 1; // 要读取的实际字节数
                    // 实际读走这一行（把socket缓冲区“消费掉”）
                    if (readn(pstr, sz) != sz)
                        return -1;
                    pstr += sz;
                    total += sz;
                    *pstr = '\0';
                    return total;
                }
            }
            // 4. 没有找到\n，把peek到的全都读出来
            if (readn(pstr, ret) != ret)
                return -1;
            pstr += ret;
            left -= ret;
            total += ret;
        }
    }
    *pstr = '\0';
    return total;
}

/**
 * @param buf
 * @param len
 * @return int
 */
int SocketIO::writen(const char *buf, int len)
{
    int left = len; // 剩余要读取的字节
    const char *pstr = buf;
    while (left > 0)
    {
        int nwrite = write(m_fd, pstr, left);
        if (nwrite == 0)
        {
            break; // 对端关闭
        }
        else if (-1 == nwrite && errno == EINTR) // 对当前进程（线程）而言发生了中断//中断触发，导致CPU的使用权被抢夺，后续应该继续执行
        {
            continue;
        }
        else if (-1 == nwrite)
        {
            return -1;
        }
        else
        {
            left -= nwrite;
            pstr += nwrite;
        }
    }
    return len - left;
}
