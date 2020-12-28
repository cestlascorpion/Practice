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
    bool Echo(uint32_t uid, const string &request, uint32_t &rid, string &response) {
        EchoRequest req;
        req.set_uid(uid);
        req.set_content(request);

        EchoResponse resp;

        ::grpc::ClientContext context;
        ::grpc::CompletionQueue cq;
        ::grpc::Status status;

        unique_ptr<::grpc::ClientAsyncResponseReader<EchoResponse>> rpc(stub_->PrepareAsyncEcho(&context, req, &cq));
        rpc->StartCall();

        rpc->Finish(&resp, &status, (void *)1);
        void *got_tag;
        bool ok = false;

        GPR_ASSERT(cq.Next(&got_tag, &ok));
        GPR_ASSERT(got_tag == (void *)1);
        GPR_ASSERT(ok);

        if (status.ok()) {
            rid = resp.uid();
            response.swap(*resp.mutable_content());
            return true;
        } else {
            return false;
        }
    }

private:
    unique_ptr<EchoService::Stub> stub_;
};

} // namespace example

int main() {
    cout << "hello, World!" << endl;
    string address = "localhost:16001";
    example::ExmapleClient client(::grpc::CreateChannel(address, ::grpc::InsecureChannelCredentials()));

    for (int i = 0; i < 5; ++i) {
        uint32_t rid = 0;
        string response;
        bool ok = client.Echo(1234 + i, "hello world", rid, response);
        if (ok) {
            cout << "rid: " << rid << " content: " << response << endl;
        }
        this_thread::sleep_for(chrono::seconds(2));
    }
}