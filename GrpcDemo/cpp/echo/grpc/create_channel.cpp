#include "create_channel.h"
#include "unary_interceptor.h"

#include <chrono>
#include <unistd.h>

#include <grpc/impl/codegen/grpc_types.h>
#include <grpc/support/log.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

using namespace std;
using namespace chrono;

using namespace grpc;
using namespace experimental;

static void grpc_log(gpr_log_func_args *args) {
    switch (args->severity) {
    case GPR_LOG_SEVERITY_DEBUG:
        printf("[GRPC::DEBUG] [%s:%d] %s\n", args->file, args->line, args->message);
        break;
    case GPR_LOG_SEVERITY_INFO:
        printf("[GRPC::INFO] [%s:%d] %s\n", args->file, args->line, args->message);
        break;
    case GPR_LOG_SEVERITY_ERROR:
        printf("[GRPC::ERROR] [%s:%d] %s\n", args->file, args->line, args->message);
        break;
    }
}

static bool enable_grpc_egress() {
    static const char *k_enable_grpc_egress = getenv("ENABLE_GRPC_EGRESS");
    return k_enable_grpc_egress != nullptr && strcasecmp(k_enable_grpc_egress, "true") == 0;
}

static string grpc_egress_address() {
    static string grpc_egress_address;
    static once_flag once;
    call_once(once, [&]() {
        const char *address = getenv("GRPC_EGRESS_ADDRESS");
        if (address != nullptr && strlen(address) > 0) {
            grpc_egress_address = address;
        } else {
            static const char *k_default_grpc_egress_address = "unix:///var/run/grpc-egress.sock";
            grpc_egress_address = k_default_grpc_egress_address;
        }
    });
    return grpc_egress_address;
}

class GrpcClientContextCallbacks : public ClientContext::GlobalCallbacks {
public:
    ~GrpcClientContextCallbacks() override {
        printf("[%s] construct\n", __func__);
    };

    void DefaultConstructor(ClientContext *context) override {
        printf("[%s] default construct\n", __func__);

        static const milliseconds default_unary_call_timeout(3000);
        context->set_wait_for_ready(false);
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

        setenv("GRPC_DNS_RESOLVER", "ares", 1);

        const char *severity = getenv("GRPC_VERBOSITY");
        if (severity != nullptr) {
            printf("InitGrpcExtension: log severity %s\n", severity);
        }

        gpr_set_log_function(grpc_log);
    }
};

shared_ptr<Channel> CreateInsecureChannelToEgress(const string &target) {
    printf("[%s] channel to egress\n", __func__);

    string authority = target;
    auto last_slash = authority.find_last_of('/');
    if (last_slash != string::npos) {
        authority = authority.substr(last_slash + 1);
    }

    const auto address = grpc_egress_address();
    printf("connect to %s with authority %s\n", address.c_str(), authority.c_str());

    ChannelArguments args;
    args.SetInt(GRPC_ARG_MAX_CONNECTION_IDLE_MS, INT32_MAX);
    args.SetInt(GRPC_ARG_MAX_CONNECTION_AGE_MS, INT32_MAX);
    args.SetString(GRPC_ARG_DEFAULT_AUTHORITY, authority);
    args.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 5000);
    args.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 3000);
    args.SetInt(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, -1);
    args.SetInt(GRPC_ARG_USE_LOCAL_SUBCHANNEL_POOL, 1);
    args.SetString(GRPC_ARG_SECONDARY_USER_AGENT_STRING,
                   "(pid=" + to_string(getpid()) + "; tid=" + to_string(pthread_self()) + ")");

    shared_ptr<ChannelCredentials> credentials = InsecureChannelCredentials();
    vector<unique_ptr<ClientInterceptorFactoryInterface>> factories;
    factories.emplace_back(new grpc_ext::UnaryInterceptorFactory());

    return CreateCustomChannelWithInterceptors(address, InsecureChannelCredentials(), args, move(factories));
}

namespace grpc_ext {

shared_ptr<Channel> CreateInsecureChannel(const string &target, bool *egress_enabled_ptr) {
    printf("[%s] create channel\n", __func__);

    static GrpcEnvSetup grpc_env_setup;

    shared_ptr<Channel> out;
    bool egress_enabled = false;
    if (enable_grpc_egress()) {
        egress_enabled = true;
        out = CreateInsecureChannelToEgress(target);
    } else {
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
        factories.emplace_back(new UnaryInterceptorFactory());
        out = CreateCustomChannelWithInterceptors(target, InsecureChannelCredentials(), args, move(factories));
    }

    if (egress_enabled) {
        *egress_enabled_ptr = egress_enabled;
    }

    return out;
}

} // namespace grpc_ext
