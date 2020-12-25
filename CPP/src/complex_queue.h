#ifndef SPIDER_COMPLEX_QUEUE_H
#define SPIDER_COMPLEX_QUEUE_H

#include <condition_variable>
#include <memory>
#include <mutex>

template <typename T>
class complex_queue {
private:
    struct node {
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;
    };

    std::unique_ptr<node> head;
    node *tail;

    std::mutex head_mtx;
    std::mutex tail_mtx;
    std::condition_variable cond;

    node *get_tail() {
        std::lock_guard<std::mutex> lock(tail_mtx);
        return tail;
    }

    std::unique_ptr<node> pop_head() {
        std::unique_ptr<node> old_head = std::move(head);
        head = std::move(old_head->next);
        return old_head;
    }

    std::unique_lock<std::mutex> wait_for_data() {
        std::unique_lock<std::mutex> lock(head_mtx);
        cond.wait(lock, [&]() -> bool { return head.get() != get_tail(); });
        return std::move(lock);
    }

    std::unique_ptr<node> wait_pop_head() {
        std::unique_lock<std::mutex> lock(wait_for_data());
        return pop_head();
    }

    std::unique_ptr<node> wait_por_head(T &val) {
        std::unique_lock<std::mutex> lock(wait_for_data());
        val = std::move(*head->data);
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head() {
        std::lock_guard<std::mutex> lock(head_mtx);
        if (head.get() == get_tail()) {
            return std::shared_ptr<node>();
        }
        return pop_head();
    }

    std::unique_ptr<node> try_pop_head(T &val) {
        std::lock_guard<std::mutex> lock(head_mtx);
        if (head.get() == get_tail()) {
            return std::shared_ptr<node>();
        }
        val = std::move(*head->data);
        return pop_head();
    }

public:
    complex_queue()
        : head(new node)
        , tail(head.get()){};

    ~complex_queue() = default;

    complex_queue(const complex_queue &) = delete;
    complex_queue &operator=(const complex_queue &) = delete;

    std::shared_ptr<T> wait_and_pop() {
        std::unique_ptr<node> old_head = wait_pop_head();
        return old_head->data;
    }

    void wait_and_pop(T &val) {
        std::unique_ptr<node> old_head = wait_pop_head(val);
    }

    std::shared_ptr<T> try_pop() {
        std::unique_ptr<node> old_head = try_pop_head();
        return old_head ? old_head->data : std::shared_ptr<T>();
    }

    bool try_pop(T &val) {
        std::unique_ptr<node> old_head = try_pop_head(val);
        return old_head;
    }

    void push(T val) {
        std::shared_ptr<T> new_data(std::make_shared<T>(std::move(val)));
        std::unique_ptr<node> p(new node);
        node *new_tail = p.get();
        {
            std::lock_guard<std::mutex> lock(tail_mtx);
            tail->data = new_data;
            tail->next = std::move(p);
            tail = new_tail;
        }
        cond.notify_one();
    }

    void empty() {
        std::lock_guard<std::mutex> lock(head_mtx);
        return (head.get() == get_tail());
    }
};

#endif // SPIDER_COMPLEX_QUEUE_H