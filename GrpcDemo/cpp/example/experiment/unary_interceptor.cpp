#include "unary_interceptor.h"

#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>

#include <string>

using namespace std;
using namespace chrono;

using namespace grpc;
using namespace experimental;

class UnaryClientInterceptor : public Interceptor {
public:
    explicit UnaryClientInterceptor(ClientRpcInfo *info)
        : info_(info) {
        printf("[%s] construct\n", __func__);
    }
    ~UnaryClientInterceptor() override {
        printf("[%s] deconstruct\n", __func__);
    }

public:
    void Intercept(InterceptorBatchMethods *methods) override {
        printf("[%s] Intercept override\n", __func__);

        if (info_->type() != ClientRpcInfo::Type::UNARY) {
            return;
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::PRE_SEND_INITIAL_METADATA)) {
            printf("hook point PRE_SEND_INITIAL_METADATA\n");
        }
        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::PRE_SEND_MESSAGE)) {
            printf("hook point PRE_SEND_MESSAGE\n");
        }
        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::POST_SEND_MESSAGE)) {
            printf("hook point POST_SEND_MESSAGE\n");
        }
        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::POST_RECV_INITIAL_METADATA)) {
            printf("hook point POST_RECV_INITIAL_METADATA\n");
        }
        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::POST_RECV_MESSAGE)) {
            printf("hook point POST_RECV_MESSAGE\n");
        }
        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::POST_RECV_STATUS)) {
            printf("hook point POST_RECV_STATUS\n");
        }
        methods->Proceed();
    }

private:
    ClientRpcInfo *info_;
};

namespace grpc_ext {

Interceptor *UnaryInterceptorFactory::CreateClientInterceptor(ClientRpcInfo *info) {
    return new UnaryClientInterceptor(info);
}

} // namespace grpc_ext
