#include "echo_grpc_client.h"

#include <string>

using namespace std;

int main() {
    auto client = echo::shared_client();

    uint32_t uid = 0;
    string response;

    auto ret = client->Echo(1234u, "hello world", uid, response);
    if (ret == 0) {
        printf("response: %u %s\n", uid, response.c_str());
    }
}