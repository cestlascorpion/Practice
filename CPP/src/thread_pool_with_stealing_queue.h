#ifndef SPIDER_THREAD_POOL_WITH_STEALING_QUEUE_H
#define SPIDER_THREAD_POOL_WITH_STEALING_QUEUE_H

#include "join_threads.h"
#include "thread_safe_queue.h"
#include <atomic>
#include <future>
#include <memory>
#include <queue>
#include <vector>

template <typename task_type>
class work_stealing_queue {
public:
    work_stealing_queue() = default;
    work_stealing_queue(const work_stealing_queue &) = delete;
    work_stealing_queue &operator=(const work_stealing_queue &) = delete;

    void push(task_type data) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push_front(std::move(data));
    }

    bool try_pop(task_type &data) {
        std::lock_guard<std::mutex> lock(mutex);
        if (queue.empty()) {
            return false;
        }
        data = std::move(queue.front());
        queue.pop_front();
        return true;
    }

    bool try_steal(task_type &data) {
        std::lock_guard<std::mutex> lock(mutex);
        if (queue.empty()) {
            return false;
        }
        data = std::move(queue.back());
        queue.pop_back();
        return true;
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

private:
    std::deque<task_type> queue;
    mutable std::mutex mutex;
};

template <typename task_type>
class thread_pool_with_stealing_queue {
private:
    std::atomic<bool> done;
    thread_safe_queue<task_type> pool_work_queue;
    std::vector<std::unique_ptr<work_stealing_queue<task_type>>> queues;
    std::vector<std::thread> threads;
    join_threads joiner; // 必须放在threads之后

    static thread_local work_stealing_queue<task_type> *local_work_queue;
    static thread_local unsigned my_index;

private:
    void worker_thread(unsigned index) {
        my_index = index;
        local_work_queue = queues[index].get();
        while (!done) {
            run_pending_task();
        }
    }

public:
    thread_pool_with_stealing_queue()
        : done(false)
        , joiner(threads) {
        const auto count = std::thread::hardware_concurrency();
        try {
            for (unsigned long i = 0; i < count; ++i) {
                queues.push_back(std::make_unique<work_stealing_queue<task_type>>());
                threads.emplace_back(&thread_pool_with_stealing_queue::worker_thread, this, i);
            }
        } catch (...) {
            done = true;
            throw;
        }
    }

    ~thread_pool_with_stealing_queue() {
        done = true;
    }

    bool pop_task_from_local_queue(task_type &task) {
        return local_work_queue && local_work_queue->try_pop(task);
    }

    bool pop_task_from_pool_queue(task_type &task) {
        return pool_work_queue.try_pop(task);
    }

    bool pop_task_from_other_queue(task_type &task) {
        for (unsigned i = 0; i < queues.size(); ++i) {
            const auto index = (my_index + 1) % queues.size();
            if (queues[index]->try_steal(task)) {
                return true;
            }
        }
        return false;
    }

    void run_pending_task() {
        task_type task;
        if (pop_task_from_local_queue(task) || pop_task_from_pool_queue(task) || pop_task_from_other_queue(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }

    template <typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f) {
        using result_type = typename std::result_of<FunctionType()>::type;

        std::packaged_task<result_type()> task(f);
        std::future<result_type> res(task.get_future());
        if (local_work_queue) {
            local_work_queue->push(std::move(task));
        } else {
            pool_work_queue.push(std::move(task));
        }
        return res;
    }
};

#endif // SPIDER_THREAD_POOL_WITH_STEALING_QUEUE_H
