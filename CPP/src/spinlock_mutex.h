#ifndef SPIDER_SPINLOCK_MUTEX_H
#define SPIDER_SPINLOCK_MUTEX_H

#include <atomic>

class spinlock_mutex {
public:
    spinlock_mutex()
        : flag(ATOMIC_FLAG_INIT) {}
    ~spinlock_mutex() = default;

public:
    void lock() {
        while (flag.test_and_set(std::memory_order::memory_order_acquire)) {
        }
    }
    void unlock() {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag;
};

#endif // SPIDER_SPINLOCK_MUTEX_H
