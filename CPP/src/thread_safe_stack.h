#ifndef SPIDER_THREAD_SAFE_STACK_H
#define SPIDER_THREAD_SAFE_STACK_H

#include <exception>
#include <memory>
#include <mutex>
#include <stack>

template <typename T>
class thread_safe_stack {
public:
    thread_safe_stack() = default;
    ~thread_safe_stack() = default;

    thread_safe_stack(const thread_safe_stack &other) {
        std::lock_guard<std::mutex> lock(other.m_mtx);
        m_data = other.m_data;
    };
    thread_safe_stack &operator=(const thread_safe_stack &) = delete;

    void push(T val) {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_data.push(std::move(val));
    }

    std::shared_ptr<T> pop() {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_data.empty()) {
            return std::shared_ptr<T>();
        }

        const std::shared_ptr<T> res(std::make_shared<T>(std::move(m_data.top())));
        m_data.pop();
        return res;
    }

    bool pop(T &val) {
        std::lock_guard<std::mutex> lock(m_mtx);
        if (m_data.empty()) {
            return false;
        }

        val = std::move(m_data.top());
        m_data.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_data.empty();
    }

private:
    std::stack<T> m_data;
    mutable std::mutex m_mtx;
};

#endif // SPIDER_THREAD_SAFE_STACK_H