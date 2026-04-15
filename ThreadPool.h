/**
 * Project 66th
 */

#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include "TaskQueue.h"

#include <atomic>
#include <cstddef>
#include <thread>
#include <vector>

class ThreadPool
{
public:
    /**
     * @param threadNum
     * @param queSize
     */
    ThreadPool(size_t threadNum, size_t queSize);

    ~ThreadPool();

    void start();

    void stop();

    /**
     * @param ptask
     */
    void addTask(ElemType &&ptask);

private:
    size_t m_threadNum;
    std::vector<std::thread> m_threads;
    size_t m_queSize;
    TaskQueue m_que;
    std::atomic<bool> m_isExit;

    ElemType getTask();

    void doTask();
};

#endif //_THREADPOOL_H
