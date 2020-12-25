#ifndef SPIDER_JOIN_THREADS_H
#define SPIDER_JOIN_THREADS_H

#include <memory>
#include <thread>
#include <vector>

class join_threads {
public:
    explicit join_threads(std::vector<std::thread> &_threads)
        : threads(_threads) {}

    ~join_threads() {
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

private:
    std::vector<std::thread> &threads;
};

#endif // SPIDER_JOIN_THREADS_H
