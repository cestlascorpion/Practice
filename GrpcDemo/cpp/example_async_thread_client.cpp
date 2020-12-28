#include <chrono>
#include <iostream>
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
        EchoRequest req;
        req.set_uid(uid);
        req.set_content(request);

        AsyncClientCall *call = new AsyncClientCall;
        call->response_reader = stub_->PrepareAsyncEcho(&call->context, req, &cq_);
        call->response_reader->StartCall();
        call->response_reader->Finish(&call->resp, &call->status, (void *)call);
    }

    void AsyncCompleteRpc() {
        void *got_tag;
        bool ok = false;

        while (cq_.Next(&got_tag, &ok)) {
            AsyncClientCall *call = static_cast<AsyncClientCall *>(got_tag);
            GPR_ASSERT(ok);
            if (call->status.ok()) {
                printf("rid: %u content: %s\n", call->resp.uid(), call->resp.content().c_str());
            }
            delete call;
        }
    }

private:
    struct AsyncClientCall {
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
    cout << "hello, World!" << endl;
    string address = "localhost:16001";
    example::ExmapleClient client(::grpc::CreateChannel(address, ::grpc::InsecureChannelCredentials()));
    thread thread_ = thread(&example::ExmapleClient::AsyncCompleteRpc, &client);

    for (int i = 0; i < 5; ++i) {
        client.Echo(1234 + i, "hello world");
        this_thread::sleep_for(chrono::seconds(2));
    }

    cout << "Press CTRL-C to quit" << endl;
    thread_.join();
    return 0;
}