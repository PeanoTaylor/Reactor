/**
 * Project 66th
 */

#include "TaskQueue.h"

/**
 * TaskQueue implementation
 */

/**
 * @param capa
 */
TaskQueue::TaskQueue(size_t capa) : m_capacity(capa), m_flag(true)
{
}

TaskQueue::~TaskQueue()
{
}

// 生产者线程会调用push函数
void TaskQueue::push(ElemType &&value)
{
    // 1.先上锁
    std::unique_lock<std::mutex> ul(m_mtx);

    // 2.判满
    while (m_que.size() == m_capacity && m_flag) // 防止虚假唤醒
    {
        // 如果TaskQueue是满的，就应该让
        // 生产者在条件变量上等待
        m_notFull.wait(ul); // 生产者阻塞，等待消费者消费任务。
        // 底层原理是：让生产者线程阻塞，
        // 会先释放锁，等到被唤醒时再重新持有锁
    }

    if (!m_flag)
    {
        return;
    }

    // 如果TaskQueue不是满的，
    // 生产者线程就可以将生产的数据加入TaskQueue
    m_que.push(std::move(value));

    // 唤醒消费者，可以开始消费仓库中的数据
    m_notEmpty.notify_one();

    // 3.解锁（自动进行）
}

/**
 * @return int
 */
// 消费者线程会调用pop函数
ElemType TaskQueue::pop()
{
    std::unique_lock<std::mutex> ul(m_mtx);

    // 因为此处仅仅以任务队列是否为空作为判断条件
    // 所以最后线程池退出时唤醒了子线程
    // 又会马上陷入阻塞
    //
    // 任务队列为空，且线程池不退出，那么应该阻塞子线程
    // 任务队列为空，且线程池退出，那么不应该阻塞子线程
    while (m_que.empty() && m_flag)
    {
        m_notEmpty.wait(ul); // 消费者阻塞，等待生产者放入任务。
    }

    if (!m_que.empty())
    {
        ElemType temp = std::move(m_que.front());
        m_que.pop();

        // 唤醒生产者
        m_notFull.notify_one();

        return temp;
    }
    else
    {
        return nullptr;
    }
}

/**
 * @return bool
 */
bool TaskQueue::full()
{
    std::lock_guard<std::mutex> guard(m_mtx);
    return m_que.size() == m_capacity;
}

/**
 * @return bool
 */
bool TaskQueue::empty()
{
    std::lock_guard<std::mutex> guard(m_mtx);
    return m_que.empty();
}

/**
 * @return void
 */
void TaskQueue::wakeup()
{
    {
        std::lock_guard<std::mutex> guard(m_mtx);
        m_flag = false;
    }
    m_notEmpty.notify_all(); // 程序要退出、队列状态整体变化时
    m_notFull.notify_all();
}
