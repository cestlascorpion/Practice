#ifndef SPIDER_THREAD_SAFE_LIST_H
#define SPIDER_THREAD_SAFE_LIST_H

#include <algorithm>
#include <mutex>

template <typename T>
class thread_safe_list {
private:
    struct node {
        std::mutex mutex;
        std::shared_ptr<T> data;
        std::unique_ptr<node> next;

        node() = default;
        explicit node(T const &val)
            : data(std::make_shared<T>(val)) {}
    };

    node head;

public:
    thread_safe_list() = default;
    ~thread_safe_list() {
        remove_if([](node const &) { return true; });
    }

    thread_safe_list(thread_safe_list const &) = delete;
    thread_safe_list &operator=(thread_safe_list const &) = delete;

    void push_front(T const &val) {
        std::unique_ptr<T> new_node(new node(val));
        std::lock_guard<std::mutex> lock(head.mutex);
        new_node->next = std::move(head.next);
        head.next = std::move(new_node);
    }

    template <typename Function>
    void for_each(Function f) {
        node *current = &head;
        std::unique_lock<std::mutex> lock(head.mutex);
        while (node *const next = current->next.get()) {
            std::unique_lock<std::mutex> next_lock(next->mutex);
            lock.unlock();
            f(*next->data);
            current = next;
            lock = std::move(next_lock);
        }
    }

    template <typename Predicate>
    std::shared_ptr<T> find_first_if(Predicate p) {
        node *current = &head;
        std::unique_lock<std::mutex> lock(head.mutex);
        while (node *const next = current->next.get()) {
            std::unique_lock<std::mutex> next_lock(next->mutex);
            lock.unlock();
            if (p(*next->data)) {
                return next->data;
            }
            current = next;
            lock = std::move(next_lock);
        }
        return std::shared_ptr<T>();
    }

    template <typename Predicate>
    void remove_if(Predicate p) {
        node *current = &head;
        std::unique_lock<std::mutex> lock(head.mutex);
        while (node *const next = current->next.get()) {
            std::unique_lock<std::mutex> next_lock(next->mutex);
            if (p(*next->data)) {
                std::unique_ptr<node> old_next = std::move(current->next);
                current->next = std::move(next->next);
                next_lock.unlock();
            } else {
                lock.unlock();
                current = next;
                lock = std::move(next_lock);
            }
        }
    }
};

#endif // SPIDER_THREAD_SAFE_LIST_H
