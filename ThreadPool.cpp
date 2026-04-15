/**
 * Project 66th
 */

#include "ThreadPool.h"

#include <utility>

/**
 * ThreadPool implementation
 */

/**
 * @param threadNum
 * @param queSize
 */
ThreadPool::ThreadPool(size_t threadNum, size_t queSize) : m_threadNum(threadNum), m_threads(), m_queSize(queSize), m_que(m_queSize), m_isExit(false)
{
}

ThreadPool::~ThreadPool()
{
    if (!m_isExit.load())
    {
        stop();
    }
}

/**
 * @return void
 */
void ThreadPool::start()
{
    m_threads.reserve(m_threadNum);
    for (size_t idx = 0; idx < m_threadNum; ++idx)
    {
        // 线程池启动后，需要创建m_threadNum个子线程备用
        // 但是往vector中进行push_back时不能传左值thread
        // 因为这样会触发thread的拷贝构造，
        // 而thread拷贝构造被删除
        //
        /* thread th(&ThreadPool::doTask,this); */
        /* m_threads.push_back(std::move(th)); */

        m_threads.emplace_back(&ThreadPool::doTask, this);
    }
}

/**
 * @return void
 */
void ThreadPool::stop()
{
    const bool was_exit = m_isExit.exchange(true);
    if (was_exit)
    {
        return;
    }
    m_que.wakeup();

    // 让主线程等待子线程退出
    for (auto &th : m_threads)
    {
        if (th.joinable())
        {
            th.join();
        }
    }
}

/**
 * @param ptask
 * @return void
 */
void ThreadPool::addTask(ElemType &&ptask)
{
    // 因为如果出现空指针调用虚函数的情况
    // 一定会发生段错误，所以做一个检查
    if (ptask && !m_isExit.load())
    {
        // 生产者线程把任务往TaskQueue里面加
        // 加锁
        // 判满，
        // 如果满就阻塞
        // 如果不满就push，并且唤醒阻塞的消费者线程（子线程）
        m_que.push(std::move(ptask)); // 已经实现
    }
}

/**
 * @return ElemType
 */
ElemType ThreadPool::getTask()
{
    return m_que.pop(); // 已经实现
}

/**
 * @return void
 */
void ThreadPool::doTask()
{
    while (true)
    {
        ElemType ptask = getTask();
        if (!ptask)
        {
            break;
        }

        ptask();
    }
}
