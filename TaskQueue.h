/**
 * Project 66th
 */

#ifndef _TASKQUEUE_H
#define _TASKQUEUE_H
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>

using ElemType = std::function<void()>;

class TaskQueue
{
public:
    /**
     * @param capa
     */
    TaskQueue(size_t capa);

    ~TaskQueue();

    /**
     * @param value
     */
    void push(ElemType &&value);

    ElemType pop();

    bool full();

    bool empty();

    void wakeup();

private:
    size_t m_capacity;                            // 队列最大容量
    std::queue<ElemType> m_que;                  // 任务队列
    mutable std::mutex m_mtx;                    // 互斥锁
    std::condition_variable m_notEmpty;          // 队列不空 -> 唤醒消费者
    std::condition_variable m_notFull;           // 队列不满 -> 唤醒生产者
    bool m_flag;                                 // 运行标记：true=运行，false=退出
};

#endif //_TASKQUEUE_H
