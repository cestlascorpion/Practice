#ifndef SPIDER_BLOCKING_QUEUE_H
#define SPIDER_BLOCKING_QUEUE_H

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
#include <mutex>
#include <vector>

class blocking_queue_with_list {
public:
    struct node {
        int idx;

        node()
            : idx(0){};
        explicit node(int id)
            : idx(id) {}
    };

public:
    explicit blocking_queue_with_list(size_t cap)
        : capacity(cap) {}

    ~blocking_queue_with_list() = default;

    blocking_queue_with_list(const blocking_queue_with_list &) = delete;

    blocking_queue_with_list &operator=(const blocking_queue_with_list &) = delete;

public:
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

    void push(std::unique_ptr<node> data) {
        std::unique_lock<std::mutex> lock(mutex);
        not_full.wait(lock, [this]() -> bool { return queue.size() < capacity; });
        queue.push_back(std::move(data));
        not_empty.notify_one();
    }

    std::unique_ptr<node> pop() {
        std::unique_lock<std::mutex> lock(mutex);
        not_empty.wait(lock, [this]() -> bool { return !queue.empty(); });
        auto res(std::move(queue.front()));
        queue.pop_front();
        not_full.notify_one();
        return res;
    }

private:
    const size_t capacity;
    std::list<std::unique_ptr<node>> queue;
    mutable std::mutex mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;
};

class blocking_queue_with_vector {
public:
    struct node {
        int idx;

        node()
            : idx(0){};
        explicit node(int id)
            : idx(id) {}
    };

public:
    explicit blocking_queue_with_vector(size_t cap)
        : queue(cap)
        , head(0)
        , tail(0) {}

    ~blocking_queue_with_vector() = default;

    blocking_queue_with_vector(const blocking_queue_with_vector &) = delete;

    blocking_queue_with_vector &operator=(const blocking_queue_with_vector &) = delete;

public:
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return head == tail;
    }

    void push(std::unique_ptr<node> data) {
        std::unique_lock<std::mutex> lock(mutex);
        not_full.wait(lock, [this]() -> bool { return (tail + 1) % queue.size() != head; });
        queue[tail] = std::move(data);
        tail = (tail + 1) % queue.size();
        not_empty.notify_one();
    }

    std::unique_ptr<node> pop() {
        std::unique_lock<std::mutex> lock(mutex);
        not_empty.wait(lock, [this]() -> bool { return head != tail; });
        auto res(std::move(queue.front()));
        head = (head + 1) % queue.size();
        not_full.notify_one();
        return res;
    }

private:
    std::vector<std::unique_ptr<node>> queue;
    std::vector<std::unique_ptr<node>>::size_type head;
    std::vector<std::unique_ptr<node>>::size_type tail;
    mutable std::mutex mutex;
    std::condition_variable not_empty;
    std::condition_variable not_full;
};

template <typename T>
class blocking_queue_with_atomic {
public:
    struct node {
        std::unique_ptr<T> value;
        node *next;

        node() = default;
        explicit node(std::unique_ptr<T> v)
            : value(std::move(v))
            , next(nullptr) {}
    };

public:
    explicit blocking_queue_with_atomic(size_t cap)
        : head(new node)
        , tail(head)
        , capacity(cap)
        , size(0) {}
    ~blocking_queue_with_atomic() {
        while (try_pop(std::make_unique<T>())) {
        }
        delete head;
    }
    blocking_queue_with_atomic(const blocking_queue_with_atomic &) = delete;
    blocking_queue_with_atomic &operator=(const blocking_queue_with_atomic &) = delete;

    void push(std::unique_ptr<T> value) {
        std::unique_lock<std::mutex> lock(tail_mtx);
        not_full.wait(lock, [this]() { return size.load() < capacity; });
        tail->value = std::move(value);
        node *new_tail = new node;
        tail->next = new_tail;
        tail = new_tail;
        lock.unlock();

        size.fetch_add(1);
        not_empty.notify_one();
    }

    std::unique_ptr<T> pop() {
        std::unique_lock<std::mutex> lock(head_mtx);
        not_empty.wait(lock, [this]() { return head != get_tail(); });
        node *old_head = head;
        auto value = std::move(old_head->value);
        head = old_head->next;
        lock.unlock();

        size.fetch_sub(1);
        not_full.notify_one();
        delete old_head;
        return value;
    }

    bool try_pop(std::unique_ptr<T> value) {
        std::unique_lock<std::mutex> lock(head_mtx);
        if (head == get_tail()) {
            return false;
        }

        node *old_head = head;
        value = std::move(head->value);
        head = old_head->next;
        lock.unlock();

        size.fetch_sub(1);
        not_full.notify_one();
        delete old_head;
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(head_mtx);
        return head == get_tail();
    }

private:
    node *get_tail() const {
        std::lock_guard<std::mutex> lock(tail_mtx);
        return tail;
    }

private:
    node *head;
    node *tail;
    const size_t capacity;
    std::atomic<size_t> size;
    mutable std::mutex head_mtx;
    mutable std::mutex tail_mtx;
    std::condition_variable not_empty;
    std::condition_variable not_full;
};

#endif // SPIDER_BLOCKING_QUEUE_H
