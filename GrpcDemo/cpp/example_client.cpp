#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include <grpcpp/grpcpp.h>

#include "example.grpc.pb.h"
#include "example.pb.h"

using namespace std;

namespace example {

class EchoClient {
public:
    EchoClient(shared_ptr<::grpc::Channel> channel)
        : stub_(EchoService::NewStub(channel)) {}

public:
    bool Echo(uint32_t uid, const string &request, uint32_t &rid, string &response) {
        EchoRequest req;
        req.set_uid(uid);
        req.set_content(request);

        EchoResponse resp;

        ::grpc::ClientContext context;
        ::grpc::Status status = stub_->Echo(&context, req, &resp);

        if (status.ok()) {
            printf("rpc succeed\n");
            rid = resp.uid();
            response.swap(*resp.mutable_content());
            return true;
        } else {
            printf("rpc fail. %d %s\n", status.error_code(), status.error_message().c_str());
            return false;
        }
    }

private:
    std::unique_ptr<EchoService::Stub> stub_;
};

} // namespace example

int main() {
    cout << "hello, World!" << endl;
    string address = "localhost:16001";
    example::EchoClient client(::grpc::CreateChannel(address, ::grpc::InsecureChannelCredentials()));

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