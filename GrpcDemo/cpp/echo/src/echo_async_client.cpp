#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include "echo.grpc.pb.h"
#include "echo.pb.h"

using namespace std;

namespace echo {

class ExampleClient {
public:
    explicit ExampleClient(shared_ptr<::grpc::Channel> channel)
        : stub_(EchoService::NewStub(channel)) {}

public:
    bool Echo(uint32_t inId, const string &request, uint32_t &outId, string &response) {
        // 1. 构建 request
        EchoRequest req;
        req.set_uid(inId);
        req.set_content(request);
        // 2. 准备 response 和 context/cq/status
        EchoResponse resp;
        ::grpc::ClientContext context;
        ::grpc::CompletionQueue cq;
        ::grpc::Status status;
        // 3. 准备 rpc 异步请求的返回结果结构体
        unique_ptr<::grpc::ClientAsyncResponseReader<EchoResponse>> rpc(stub_->PrepareAsyncEcho(&context, req, &cq));
        // 4. 调用并指定返回结果的 tag（经过cq传递的一个真实的数据结构？）
        rpc->StartCall();
        rpc->Finish(&resp, &status, (void *)1);
        // 5. 取回调用结果
        void *got_tag;
        bool ok = false;
        GPR_ASSERT(cq.Next(&got_tag, &ok)); // 返回队列
        GPR_ASSERT(got_tag == (void *)1);   // 检查 tag
        GPR_ASSERT(ok);
        // 4. 检查结果
        if (status.ok()) {
            outId = resp.uid();
            response.swap(*resp.mutable_content());
            return true;
        } else {
            printf("rpc fail %d %s\n", status.error_code(), status.error_message().c_str());
            return false;
        }
    }

private:
    unique_ptr<EchoService::Stub> stub_;
};

} // namespace echo

int main() {
    string address = "localhost:16001";
    // 创建 channel - 创建调用桩
    echo::ExampleClient client(::grpc::CreateChannel(address, ::grpc::InsecureChannelCredentials()));

    for (auto i = 0u; i < 5; ++i) {
        uint32_t uid = 0;
        string response;
        // 同步调用: 使用异步方法的同步调用
        bool ok = client.Echo(1234u + i, "hello world", uid, response);
        if (ok) {
            printf("uid: %u content: %s\n", uid, response.c_str());
        }
        this_thread::sleep_for(chrono::seconds(2));
    }
}