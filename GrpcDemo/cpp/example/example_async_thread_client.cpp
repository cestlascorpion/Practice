#include <chrono>
#include <memory>
#include <thread>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include "example.grpc.pb.h"
#include "example.pb.h"

using namespace std;

namespace example {

class ExmapleClient {
public:
    explicit ExmapleClient(shared_ptr<::grpc::Channel> channel)
        : stub_(EchoService::NewStub(channel)) {}

public:
    void Echo(uint32_t uid, const string &request) {
        // 1. 构建 request
        EchoRequest req;
        req.set_uid(uid);
        req.set_content(request);
        // 2. 准备 AsyncClient 初始化 cq
        AsyncClient *cli = new AsyncClient; // 一次 rpc 所需的独立的 context status
        cli->response_reader = stub_->PrepareAsyncEcho(&cli->context, req, &cq_);
        // 3. 异步调用 AsyncClient 被用作 tag
        cli->response_reader->StartCall();
        cli->response_reader->Finish(&cli->resp, &cli->status, (void *)cli);
    }

    void AsyncCompleteRpc() {
        // 从 cq 中循环取出调用结果并进行检查
        void *got_tag;
        bool ok = false;
        while (cq_.Next(&got_tag, &ok)) {
            // AsyncClient 被用作 tag
            AsyncClient *cli = static_cast<AsyncClient *>(got_tag);
            GPR_ASSERT(ok);
            if (cli->status.ok()) {
                printf("uid: %u content: %s\n", cli->resp.uid(), cli->resp.content().c_str());
            } else {
                printf("rpc fail %d %s\n", cli->status.error_code(), cli->status.error_message().c_str());
            }
            delete cli; // cli 在调用时创建 经过 cq 传递过来 最后销毁
        }
    }

private:
    struct AsyncClient {
        EchoResponse resp;
        ::grpc::ClientContext context;
        ::grpc::Status status;
        unique_ptr<::grpc::ClientAsyncResponseReader<EchoResponse>> response_reader;
    };

private:
    unique_ptr<EchoService::Stub> stub_;
    ::grpc::CompletionQueue cq_;
};

} // namespace example

int main() {
    string address = "localhost:16001";
    example::ExmapleClient client(::grpc::CreateChannel(address, ::grpc::InsecureChannelCredentials()));
    // 检查异步返回结果的线程
    thread thread_ = thread(&example::ExmapleClient::AsyncCompleteRpc, &client);
    // 异步调用
    for (int i = 0; i < 5; ++i) {
        client.Echo(1234 + i, "hello world");
        this_thread::sleep_for(chrono::seconds(2));
    }
    printf("Press CTRL-C to quit\n");
    thread_.join();
    return 0;
}