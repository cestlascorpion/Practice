#include "parallel_quick_sort.h"
#include <iostream>

using namespace std;

struct node {
    std::string word;
    int64_t score;
};

bool operator<(const node &n1, const node &n2) {
    return n1.score > n2.score;
}

int main() {
    std::list<node> data;
    for (int i = 0; i < 5000000; ++i) {
        int random = rand() % 100000;
        data.push_back({to_string(random), random});
    }

    timespec t1, t2;

    clock_gettime(CLOCK_MONOTONIC, &t1);
    data.sort();
    clock_gettime(CLOCK_MONOTONIC, &t2);
    cout << (t2.tv_nsec - t1.tv_nsec) / 1000000 + (t2.tv_sec - t1.tv_sec) * 1000 << "ms" << endl;

    return 0;
};