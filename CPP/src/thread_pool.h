#ifndef SPIDER_THREAD_POOL_H
#define SPIDER_THREAD_POOL_H

#include "function_wrapper.h"
#include "join_threads.h"
#include "thread_safe_queue.h"
#include <atomic>
#include <future>

template <typename task_type>
class thread_pool {
public:
    thread_pool()
        : done(false)
        , joiner(threads) {
        const auto count = std::thread::hardware_concurrency();
        try {
            for (unsigned long i = 0; i < count; ++i) {
                threads.emplace_back(&thread_pool::worker_thread, this);
            }
        } catch (...) {
            done = true;
            throw;
        }
    }

    ~thread_pool() {
        done = true;
    }

    void run_pending_task() {
        function_wrapper task;
        if (work_queue.try_pop(task)) {
            task();
        } else {
            std::this_thread::yield();
        }
    }

    template <typename FunctionType>
    std::future<typename std::result_of<FunctionType()>::type> submit(FunctionType f) {
        using result_type = typename std::result_of<FunctionType()>::type;

        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        work_queue.push(std::move(task));
        return res;
    }

private:
    void worker_thread() {
        while (!done) {
            function_wrapper task;
            if (work_queue.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }

private:
    std::atomic<bool> done;
    thread_safe_queue<task_type> work_queue;
    std::vector<std::thread> threads;
    join_threads joiner; // 必须放在threads之后
};

#endif // SPIDER_THREAD_POOL_H
