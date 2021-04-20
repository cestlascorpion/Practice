#include "echo_client.h"
#include "grpc_ext_base.h"

using namespace std;
using namespace grpc;

namespace example {
namespace echo {

static const char *k_default_target = "echo.svc.local:16001";

shared_ptr<Client_v1> default_client_v1() {
    static shared_ptr<Client_v1> cli(new Client_v1(k_default_target));
    return cli;
}

shared_ptr<Client_v2> default_client_v2() {
    static shared_ptr<Client_v2> cli(new Client_v2(k_default_target));
    return cli;
}

Client_v1::Client_v1(const string &target)
    : grpc_client(target) {
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

int Client_v1::Echo(uint32_t inId, const string &request, uint32_t &outId, string &response) {
    printf("[%s] Call Echo Method\n", __func__);

    EchoRequest req;
    req.set_uid(inId);
    req.set_content(request);

    auto stub = stub_.WithChannel(Channel());
    ClientContext context;

    EchoResponse resp;
    auto status = BlockingUnaryCall(
        &context,
        [&](ClientContext *ctx) {
            return grpc_ext::invoke_stub_func(ctx, stub.get(), &EchoService::StubInterface::Echo, req, &resp);
        },
        callInfo("Echo"));
    if (status.ok()) {
        outId = resp.uid();
        response.swap(*resp.mutable_content());
    }
    return int(status.error_code());
}

Status Client_v2::Echo(uint32_t inId, const string &request, uint32_t &outId, string &response) {
    printf("[%s] Call Echo Method\n", __func__);

    ClientContext cc;
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
} // namespace example