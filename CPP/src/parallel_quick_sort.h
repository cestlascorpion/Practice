#ifndef SPIDER_PARALLEL_QUICK_SORT_H
#define SPIDER_PARALLEL_QUICK_SORT_H

#include "function_wrapper.h"
#include "join_threads.h"
#include "thread_pool_with_stealing_queue.h"
#include "thread_safe_stack.h"
#include <algorithm>
#include <functional>
#include <future>
#include <list>
#include <thread>

template <typename T>
std::list<T> sequential_quick_sort(std::list<T> input) {
    if (input.empty()) {
        return input;
    }
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    const T &pivot = *result.begin();
    auto divide_point = std::partition(input.begin(), input.end(), [&](const T &t) { return t < pivot; });

    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    auto new_lower(sequential_quick_sort(std::move(lower_part)));
    auto new_higher(sequential_quick_sort(std::move(input)));

    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower);
    return result;
}

template <typename T>
std::list<T> parallel_quick_sort_async(std::list<T> input) {
    if (input.empty()) {
        return input;
    }
    std::list<T> result;
    result.splice(result.begin(), input, input.begin());
    const T &pivot = *result.begin();
    auto divide_point = std::partition(input.begin(), input.end(), [&](const T &t) { return t < pivot; });

    std::list<T> lower_part;
    lower_part.splice(lower_part.end(), input, input.begin(), divide_point);

    std::future<std::list<T>> new_lower(std::async(&parallel_quick_sort_async<T>, std::move(lower_part)));
    auto new_higher(sequential_quick_sort(std::move(input)));

    result.splice(result.end(), new_higher);
    result.splice(result.begin(), new_lower.get());
    return result;
}

template <typename T>
class sorter {
private:
    struct chunk_to_sort {
        std::list<T> data;
        std::promise<std::list<T>> promise;
    };

    thread_safe_stack<chunk_to_sort> chunks;
    std::vector<std::thread> threads;
    unsigned const max_thread_count;
    std::atomic<bool> end_of_data;

public:
    sorter()
        : max_thread_count(std::thread::hardware_concurrency() - 1)
        , end_of_data(false) {}

    ~sorter() {
        end_of_data = true;
        for (unsigned i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }
    }

    void try_sort_chunk() {
        std::shared_ptr<chunk_to_sort> chunk = chunks.pop();
        if (chunk) {
            sort_chunk(chunk);
        }
    }

    std::list<T> do_sort(std::list<T> &chunk_data) {
        if (chunk_data.empty()) {
            return chunk_data;
        }

        std::list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());
        T const &partition_val = *result.begin();

        typename std::list<T>::iterator divide_point =
            std::partition(chunk_data.begin(), chunk_data.end(), [&](T const &val) { return val < partition_val; });

        chunk_to_sort new_lower_chunk;
        new_lower_chunk.data.splice(new_lower_chunk.data.end(), chunk_data, chunk_data.begin(), divide_point);
        std::future<std::list<T>> new_lower = new_lower_chunk.promise.get_future();
        chunks.push(std::move(new_lower_chunk));

        if (threads.size() < max_thread_count) {
            threads.push_back(std::thread(&sorter<T>::sort_thread, this));
        }
        std::list<T> new_higher(do_sort(chunk_data));
        result.splice(result.end(), new_higher);
        while (new_lower.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            try_sort_chunk();
        }
        result.splice(result.begin(), new_lower.get());
        return result;
    }

    void sort_chunk(std::shared_ptr<chunk_to_sort> const &chunk) {
        chunk->promise.set_value(do_sort(chunk->data));
    }

    void sort_thread() {
        while (!end_of_data) {
            try_sort_chunk();
            std::this_thread::yield();
        }
    }
};

template <typename T>
std::list<T> parallel_quick_sort_sorter(std::list<T> input) {
    if (input.empty()) {
        return input;
    }
    sorter<T> sort;
    return sort.do_sort(input);
}

template <typename T>
class sorter_using_pool {
public:
    std::list<T> do_sort(std::list<T> &chunk_data) {
        if (chunk_data.empty()) {
            return chunk_data;
        }

        std::list<T> result;
        result.splice(result.begin(), chunk_data, chunk_data.begin());
        T const &partition_val = *result.begin();

        typename std::list<T>::iterator divide_point =
            std::partition(chunk_data.begin(), chunk_data.end(), [&](T const &val) { return val < partition_val; });

        std::list<T> new_lower_chunk{};
        new_lower_chunk.splice(new_lower_chunk.end(), chunk_data, chunk_data.begin(), divide_point);

        std::future<std::list<T>> new_lower =
            pool.submit(std::bind(&sorter_using_pool::do_sort, this, std::move(new_lower_chunk)));
        std::list<T> new_higher(do_sort(chunk_data));

        result.splice(result.end(), new_higher);
        while (new_lower.wait_for(std::chrono::seconds(0)) == std::future_status::timeout) {
            pool.run_pending_task();
        }
        result.splice(result.begin(), new_lower.get());
        return result;
    }

private:
    thread_pool_with_stealing_queue<function_wrapper> pool;
};

template <typename T>
std::list<T> parallel_quick_sort_sorter_using_pool(std::list<T> input) {
    if (input.empty()) {
        return input;
    }
    sorter_using_pool<T> sort;
    return sort.do_sort(input);
}

#endif // SPIDER_PARALLEL_QUICK_SORT_H
