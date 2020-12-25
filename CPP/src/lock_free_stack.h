#ifndef SPIDER_LOCK_FREE_STACK_H
#define SPIDER_LOCK_FREE_STACK_H

#include <atomic>
#include <memory>

// 有内存泄漏的问题
template <typename T>
class lock_free_stack {
private:
    struct node {
        std::shared_ptr<T> data;
        node *next;

        explicit node(T const &val)
            : data(std::make_shared<T>(val)) {}
    };

    std::atomic<node *> head;

public:
    void push(T const &val) {
        node *const new_node = new node(val);
        new_node->next = head.load();
        // compare_exchange_weak: 比较成功，head修改为new_node 比较失败，new_node->next修改为head.load()
        while (!head.compare_exchange_weak(new_node->next, new_node)) {
        }
    }

    std::shared_ptr<T> pop() {
        node *old_node = head.load();
        // compare_exchange_weak: 比较成功，head修改为old_node->next 比较失败，old_node修改为head.load()
        while (old_node != nullptr && !head.compare_exchange_weak(old_node, old_node->next)) {
        }
        return old_node ? old_node->data : std::shared_ptr<T>();
    }
};

#endif // SPIDER_LOCK_FREE_STACK_H
