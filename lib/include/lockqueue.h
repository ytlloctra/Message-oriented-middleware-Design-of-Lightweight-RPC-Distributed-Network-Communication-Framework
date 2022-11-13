#pragma once
#include <queue>
#include <thread>
#include <mutex>    // pthread_mutex_t  线程互斥
#include <condition_variable>   // pthread_condition_t   线程通信


// 模板代码  不能写在cc文件中
// 异步写日志的日志队列
template<typename T> 
class LockQueue {
public:
    //muduo 提供的 多个worker线程都会写日志queue
    void Push(const T &data) {
        std::lock_guard<std::mutex> lock(m_mutex);  // 获得互斥锁
        m_queue.push(data);
        m_condvariable.notify_one();  // 唤醒wait线程
    }
    // 出右括号释放锁

    // 一个线程 在读日志queue，写日志文件
    T Pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (m_queue.empty()) {
            // 日志队列为空， 线程进入wait状态
            m_condvariable.wait(lock);  // 进入wait等待状态  释放锁
        }

        T data = m_queue.front();
        m_queue.pop();
        return data;
    }
private:
    std::queue<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvariable;
};