#include "eg_client.h"
#include "create_channel.h"

using namespace std;

namespace example {

static const char *k_service_name = "echo";
static const char *k_default_target = "echo.example.local:16001";
// static const char *k_default_target = "localhost:16001";

shared_ptr<Client> default_client() {
    static shared_ptr<Client> cli(new Client(k_default_target));
    return cli;
}

Client::Client(const string &target) {
    printf("[%s] construct\n", __func__);
    auto channel = grpc_ext::CreateInsecureChannel(target);
    stub_ = EchoService::NewStub(channel);
}

Client::~Client() {
    printf("[%s] deconstruct\n", __func__);
}

grpc::Status Client::Echo(uint32_t inId, const string &request, uint32_t &outId, string &response) {
    printf("[%s] Echo Method\n", __func__);

    grpc::ClientContext cc;
    EchoRequest req;
    req.set_uid(inId);
    req.set_content(request);

    EchoResponse resp;
    auto status = stub_->Echo(&cc, req, &resp);
    if (status.ok()) {
        outId = resp.uid();
        response.swap(*resp.mutable_content());
    }
    return status;
}

} // namespace example