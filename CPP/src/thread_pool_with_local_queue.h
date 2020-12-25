#ifndef SPIDER_THREAD_POOL_WITH_LOCAL_QUEUE_H
#define SPIDER_THREAD_POOL_WITH_LOCAL_QUEUE_H

#include "join_threads.h"
#include "thread_safe_queue.h"
#include <atomic>
#include <future>
#include <memory>

template <typename task_type>
class thread_pool_with_local_queue {
private:
    static thread_local std::unique_ptr<std::queue<task_type>> local_work_queue;

    std::atomic<bool> done;
    thread_safe_queue<task_type> pool_work_queue;
    std::vector<std::thread> threads;
    join_threads joiner; // 必须放在threads之后

private:
    void worker_thread() {
        local_work_queue = std::make_unique<std::queue<task_type>>();
        while (!done) {
            run_pending_task();
        }
    }

public:
    thread_pool_with_local_queue()
        : done(false)
        , joiner(threads) {
        const auto count = std::thread::hardware_concurrency();
        try {
            for (unsigned long i = 0; i < count; ++i) {
                threads.emplace_back(&thread_pool_with_local_queue::worker_thread, this);
            }
        } catch (...) {
            done = true;
            throw;
        }
    }

    ~thread_pool_with_local_queue() {
        done = true;
    }

    void run_pending_task() {
        task_type task;
        if (local_work_queue && !local_work_queue->empty()) {
            task = std::move(local_work_queue->front());
            local_work_queue->pop();
            task();
        } else if (pool_work_queue.try_pop(task)) {
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

template <typename task_type>
thread_local std::unique_ptr<std::queue<task_type>> thread_pool_with_local_queue<task_type>::local_work_queue;

#endif // SPIDER_THREAD_POOL_WITH_LOCAL_QUEUE_H
