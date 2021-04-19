#include "echo_grpc_client.h"
#include "create_channel.h"

using namespace std;

namespace echo {

static const char *k_service = "echo";
static const char *k_default_target = "echo.svc.local:16001";

shared_ptr<Client_v1> shared_client() {
    static shared_ptr<Client_v1> cli(new Client_v1());
    return cli;
}

shared_ptr<Client_v2> default_client() {
    static shared_ptr<Client_v2> cli(new Client_v2(k_default_target));
    return cli;
}

Client_v1::Client_v1()
    : grpc_client(k_default_target, k_service)
    , stub_(k_service) {
    printf("[%s] construct\n", __func__);
}

Client_v2::Client_v2(const string &target) {
    printf("[%s] construct\n", __func__);

    auto channel = grpc_ext::CreateInsecureChannel(target);
    stub_ = EchoService::NewStub(channel);
}

Client_v1::~Client_v1() {
    printf("[%s] deconstruct\n", __func__);
}

Client_v2::~Client_v2() {
    printf("[%s] deconstruct\n", __func__);
}

int Client_v1::Echo(uint32_t inId, const std::string &request, uint32_t &outId, std::string &response) {
    printf("[%s] Echo Method\n", __func__);

    echo::EchoRequest req;
    req.set_uid(inId);
    req.set_content(request);

    auto channel = Channel();
    auto stub = stub_.WithChannel(channel);
    const char *method = __func__;
    grpc::ClientContext context;

    echo::EchoResponse resp;
    auto status = BlockingUnaryCall(
        &context,
        [&](grpc::ClientContext *ctx) {
            return grpc_ext::invoke_stub_func(method, ctx, stub.get(), &EchoService::StubInterface::Echo, req, &resp);
        },
        WithMethod(method));
    return int(status.error_code());
}

grpc::Status Client_v2::Echo(uint32_t inId, const string &request, uint32_t &outId, string &response) {
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

} // namespace echo