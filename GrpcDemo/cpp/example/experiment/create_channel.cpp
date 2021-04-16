#include "create_channel.h"
#include "unary_interceptor.h"

#include <grpc/impl/codegen/grpc_types.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

using namespace std;
using namespace chrono;

using namespace grpc;
using namespace experimental;

class GrpcClientContextCallbacks : public ClientContext::GlobalCallbacks {
public:
    ~GrpcClientContextCallbacks() override {
        printf("[%s] construct\n", __func__);
    };

    void DefaultConstructor(ClientContext *context) override {
        printf("[%s] default construct\n", __func__);
    }

    void Destructor(ClientContext *) override {
        printf("[%s] destruct\n", __func__);
    }
};

class GrpcEnvSetup final {
public:
    GrpcEnvSetup() {
        printf("[%s] construct\n", __func__);

        static GrpcClientContextCallbacks global_client_context_callbacks;
        ClientContext::SetGlobalCallbacks(&global_client_context_callbacks);
    }
};

namespace grpc_ext {

shared_ptr<Channel> CreateInsecureChannel(const string &target) {
    printf("[%s] create channel\n", __func__);

    static GrpcEnvSetup grpc_env_setup;
    ChannelArguments args;
    args.SetInt(GRPC_ARG_DNS_ENABLE_SRV_QUERIES, 1);
    args.SetInt(GRPC_ARG_DNS_ARES_QUERY_TIMEOUT_MS, 300000);
    args.SetInt(GRPC_ARG_GRPCLB_FALLBACK_TIMEOUT_MS, INT32_MAX);
    args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, 8 * 1024 * 1024);
    args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 5000);
    args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 3000);
    args.SetInt(GRPC_ARG_MAX_CONNECTION_IDLE_MS, INT32_MAX);
    args.SetInt(GRPC_ARG_MAX_CONNECTION_AGE_MS, INT32_MAX);
    args.SetInt(GRPC_ARG_USE_LOCAL_SUBCHANNEL_POOL, 1);

    vector<unique_ptr<ClientInterceptorFactoryInterface>> factories;
    factories.emplace_back(new grpc_ext::UnaryInterceptorFactory());

    return CreateCustomChannelWithInterceptors(target, InsecureChannelCredentials(), args, move(factories));
}

} // namespace grpc_ext
