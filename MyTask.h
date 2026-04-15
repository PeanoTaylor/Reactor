/**
 * Project 66th
 */

#ifndef _MYTASK_H
#define _MYTASK_H

#include "TcpConnection.h"

#include <string>

class MyTask
{
public:
    MyTask(const std::string &msg, const TcpConnectionPtr &conn)
        : m_msg(msg), m_conn(conn)
    {
    }

    void process()
    {
        const std::string response = "msg: " + m_msg;
        m_conn->sendInLoop(response);
    }

private:
    std::string m_msg;
    TcpConnectionPtr m_conn;
};

#endif //_MYTASK_H
