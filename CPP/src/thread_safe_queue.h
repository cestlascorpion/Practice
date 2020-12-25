#ifndef SPIDER_THREAD_SAFE_QUEUE_H
#define SPIDER_THREAD_SAFE_QUEUE_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

template <typename T>
class thread_safe_queue {
public:
    thread_safe_queue() = default;
    ~thread_safe_queue() = default;

    thread_safe_queue(const thread_safe_queue &other) {
        std::lock_guard<std::mutex> lock(other.m_mtx);
        m_data = other.m_data;
    };
    thread_safe_queue &operator=(const thread_safe_queue &) = delete;

    void push(T val) {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_data.push(std::move(val));
        m_cond.notify_one();
    }

    std::shared_ptr<T> pop() {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cond.wait(lock, [this]() -> bool { return !m_data.empty(); });
        std::shared_ptr<T> res(std::make_shared<T>(std::move(m_data.front())));
        m_data.pop();
        return res;
    }

    void pop(T &val) {
        std::unique_lock<std::mutex> lock(m_mtx);
        m_cond.wait(lock, [this]() -> bool { return !m_data.empty(); });
        val = std::move(m_data.front());
        m_data.pop();
    }

    std::shared_ptr<T> try_pop() {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_data.empty()) {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> res(std::make_shared<T>(std::move(m_data.front())));
        m_data.pop();
        return res;
    }

    bool try_pop(T &val) {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_data.empty()) {
            return false;
        }

        val = std::move(m_data.front());
        m_data.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_data.empty();
    }

private:
    std::queue<T> m_data;
    mutable std::mutex m_mtx;
    std::condition_variable m_cond; // push() -> notify() -> pop()
};

#endif // SPIDER_THREAD_SAFE_QUEUE_H
