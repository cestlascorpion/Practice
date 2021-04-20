#include <chrono>
#include <memory>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "echo.grpc.pb.h"
#include "echo.pb.h"

using namespace std;

namespace echo {

class EchoClient {
public:
    explicit EchoClient(shared_ptr<::grpc::Channel> channel)
        : stub_(EchoService::NewStub(channel)) {}

public:
    bool Echo(uint32_t inId, const string &request, uint32_t &outId, string &response) {
        // 1. 构建 request
        EchoRequest req;
        req.set_uid(inId);
        req.set_content(request);
        // 2. 准备 response 和 context
        EchoResponse resp;
        ::grpc::ClientContext context; // 不可重用 / rpc 结束前不可销毁
        gpr_timespec ts;
        ::grpc::TimepointHR2Timespec(chrono::system_clock::now() + chrono::seconds(3), &ts);
        context.set_deadline(ts);
        // 3. 同步调用
        ::grpc::Status status = stub_->Echo(&context, req, &resp);
        // 4. 检查结果
        if (status.ok()) {
            outId = resp.uid();
            response.swap(*resp.mutable_content()); // swap 来 move 较大的对象
        } else {
            printf("rpc fail %d %s\n", status.error_code(), status.error_message().c_str());
        }
        return status.ok();
    }

private:
    std::unique_ptr<EchoService::Stub> stub_; // 调用桩
};

} // namespace echo

int main() {
    string address = "localhost:16001";
    // 创建 channel - 创建调用桩
    echo::EchoClient client(::grpc::CreateChannel(address, ::grpc::InsecureChannelCredentials()));

    for (auto i = 0u; i < 5; ++i) {
        uint32_t uid = 0;
        string response;
        // 同步调用
        bool ok = client.Echo(1234u + i, "hello world", uid, response);
        if (ok) {
            printf("response: %u %s\n", uid, response.c_str());
        }
        this_thread::sleep_for(chrono::seconds(2));
    }
}