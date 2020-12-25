#ifndef SPIDER_THREAD_GROUP_H
#define SPIDER_THREAD_GROUP_H

#include <chrono>
#include <functional>
#include <thread>
#include <vector>

class thread_group {
public:
    thread_group(size_t count, std::function<void()> F)
        : threads(count) {
        start = std::chrono::steady_clock::now();
        for (auto &thread : threads) {
            thread = std::thread(F);
        }
    }
    ~thread_group() {
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        end = std::chrono::steady_clock::now();
        printf("using %ld \n", (end - start).count());
    }

private:
    std::vector<std::thread> threads;
    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::time_point<std::chrono::steady_clock> end;
};

#endif // SPIDER_THREAD_GROUP_H
