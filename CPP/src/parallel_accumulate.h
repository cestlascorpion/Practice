#ifndef SPIDER_PARALLEL_ACCUMULATE_H
#define SPIDER_PARALLEL_ACCUMULATE_H

#include "function_wrapper.h"
#include "thread_pool.h"
#include <future>
#include <numeric>

template <typename Iterator, typename T>
struct accumulate_block {
    T operator()(Iterator first, Iterator last) {
        return std::accumulate(first, last, 0);
    }
};

template <typename Iterator, typename T>
T parallel_accumulate(Iterator first, Iterator last, T init) {
    const unsigned long length = std::distance(first, last);
    if (!length) {
        return init;
    }

    const unsigned long block_size = 25;
    const unsigned long num_block = (length + block_size - 1) / block_size;

    std::vector<std::future<T>> futures(num_block - 1);

    thread_pool<function_wrapper> pool;
    auto block_start = first;
    for (unsigned long i = 0; i < (num_block - 1); ++i) {
        auto block_end = block_start;
        std::advance(block_end, block_size);
        futures[i] = pool.submit(std::bind(accumulate_block<Iterator, T>(), block_start, block_end));
        block_start = block_end;
    }

    T last_result = accumulate_block<Iterator, T>()(block_start, last);
    T result = init;
    for (unsigned long i = 0; i < (num_block - 1); ++i) {
        result += futures[i].get();
    }
    result += last_result;
    return result;
}

#endif // SPIDER_PARALLEL_ACCUMULATE_H
