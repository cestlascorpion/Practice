#include "eg_client.h"

#include <string>
#include <thread>

using namespace std;

int main() {
    auto client = example::default_client();
    for (auto i = 0u; i < 5; ++i) {
        uint32_t uid = 0;
        string response;
        // 同步调用
        auto status = client->Echo(1234u + i, "hello world", uid, response);
        if (status.ok()) {
            printf("response: %u %s\n", uid, response.c_str());
        }
        this_thread::sleep_for(chrono::seconds(2));
    }
}