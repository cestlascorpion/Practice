#include "../client/echo_client.h"

#include <string>

using namespace std;

int main() {
    auto client = example::echo::default_client_v2();

    uint32_t uid = 0;
    string response;

    auto status = client->Echo(1234u, "hello world", uid, response);
    if (status.ok()) {
        printf("response: %u %s\n", uid, response.c_str());
    }
}