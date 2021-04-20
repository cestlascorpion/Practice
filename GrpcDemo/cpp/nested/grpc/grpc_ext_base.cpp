#include "grpc_ext_base.h"

#include <chrono>
#include <map>
#include <string>

#include <grpc/impl/codegen/grpc_types.h>
#include <grpc/support/log.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/impl/codegen/string_ref.h>
#include <grpcpp/support/client_interceptor.h>

#include <boost/thread/shared_mutex.hpp>

using namespace std;
using namespace chrono;

using namespace grpc;
using namespace experimental;

using MetaType = multimap<string_ref, string_ref>;

static bool MetaGet(const MetaType &meta, const string_ref &key, string &value) {
    auto range = meta.equal_range(key);
    if (range.first != meta.end()) {
        value.assign(range.first->second.begin(), range.first->second.end());
        return true;
    }
    return false;
}

static bool MetaGet(const MetaType &meta, const string_ref &key, int &value) {
    string val;
    if (MetaGet(meta, key, val)) {
        value = (int)strtol(val.c_str(), nullptr, 10);
        return true;
    }
    return false;
}

static map<string, pair<string, string>> &MethodMap() {
    static map<string, pair<string, string>> methods;
    return methods;
}

static boost::shared_mutex &MethodLock() {
    static boost::shared_mutex lock;
    return lock;
}

class GlobalClientContextCallbacks : public ClientContext::GlobalCallbacks {
public:
    ~GlobalClientContextCallbacks() override {
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

class UnaryInterceptor : public Interceptor {
public:
    explicit UnaryInterceptor(ClientRpcInfo *info)
        : info_(info)
        , start_() {
        printf("[%s] construct\n", __func__);
    }
    ~UnaryInterceptor() override {
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

            start_ = high_resolution_clock::now();

            auto meta = methods->GetSendInitialMetadata();
            printf("****** ADD TRACE ID ******\n");
            meta->emplace(grpc_ext::k_ot_span_context, "trace id for test");
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::PRE_SEND_MESSAGE)) {
            printf("hook point PRE_SEND_MESSAGE\n");
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::POST_SEND_MESSAGE)) {
            printf("hook point POST_SEND_MESSAGE\n");
        }
        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::PRE_SEND_STATUS)) {
            printf("hook point PRE_SEND_STATUS\n");
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::PRE_SEND_CLOSE)) {
            printf("hook point PRE_SEND_CLOSE\n");
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::PRE_RECV_INITIAL_METADATA)) {
            printf("hook point PRE_RECV_INITIAL_METADATA\n");
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::PRE_RECV_MESSAGE)) {
            printf("hook point PRE_RECV_MESSAGE\n");
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::PRE_RECV_STATUS)) {
            printf("hook point PRE_RECV_STATUS\n");
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::POST_RECV_INITIAL_METADATA)) {
            printf("hook point POST_RECV_INITIAL_METADATA\n");
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::POST_RECV_MESSAGE)) {
            printf("hook point POST_RECV_MESSAGE\n");
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::POST_RECV_STATUS)) {
            printf("hook point POST_RECV_STATUS\n");

            auto status = methods->GetRecvStatus();
            int status_code = 0, custom_code = 0;
            string status_message, custom_message;
            if (status != nullptr) {
                status_code = status->error_code();
                status_message = status->error_message();
                if (status->ok()) {
                    InvokeMeta(*methods->GetRecvInitialMetadata(), custom_code, custom_message);
                }
            } else {
                status_code = -2;
                status_message = "invalid status";
            }
            auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start_).count();
            auto SvcAndFunc = GetMethod(info_->method());
            auto channelState = info_->channel()->GetState(false);
            printf("****** FUNC TRACK ******\n");
            printf("service %s function %s cost %ld ms ip %s(%s) status %d(%s) result %d(%s) conn %d\n",
                   get<0>(SvcAndFunc).c_str(), get<1>(SvcAndFunc).c_str(), duration,
                   info_->client_context()->peer().c_str(), info_->client_context()->debug_error_string().c_str(),
                   status_code, status_message.c_str(), custom_code, custom_message.c_str(), channelState);
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::POST_RECV_CLOSE)) {
            printf("hook point POST_RECV_CLOSE\n");
        }

        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::PRE_SEND_CANCEL)) {
            printf("hook point PRE_SEND_CANCEL\n");
        }

        methods->Proceed();
    }

private:
    static void InvokeMeta(const MetaType &meta, int &code, string &message) {
        if (MetaGet(meta, grpc_ext::k_server_error_code, code)) {
            MetaGet(meta, grpc_ext::k_server_error_message, message);
        }
    }

    static pair<string, string> GetMethod(const char *method) {
        {
            boost::shared_lock<boost::shared_mutex> lk(MethodLock());
            auto iter = MethodMap().find(method);
            if (iter != MethodMap().end()) {
                printf("find method <%s, %s>\n", get<0>(iter->second).c_str(), get<1>(iter->second).c_str());
                return iter->second;
            }
        }

        string m(method);
        printf("[%s] add method %s\n", __func__, m.c_str());
        auto pos = m.find_first_of('/', 1);
        auto svc = m.substr(1, pos - 1);
        auto func = m.substr(pos + 1);
        auto pair = make_pair(svc, func);

        {
            boost::unique_lock<boost::shared_mutex> lk;
            MethodMap().emplace(move(m), pair);
        }

        return pair;
    }

private:
    ClientRpcInfo *info_;
    high_resolution_clock::time_point start_;
};

namespace grpc_ext {

class UnaryInterceptorFactory : public ClientInterceptorFactoryInterface {
public:
    Interceptor *CreateClientInterceptor(ClientRpcInfo *info) override {
        return new UnaryInterceptor(info);
    }
};

shared_ptr<Channel> CreateInsecureChannel(const string &target) {
    printf("[%s] create channel for %s\n", __func__, target.c_str());

    static once_flag once;
    static GlobalClientContextCallbacks global_client_context_callbacks;
    call_once(once, [&]() { ClientContext::SetGlobalCallbacks(&global_client_context_callbacks); });

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

    return CreateCustomChannelWithInterceptors(target, InsecureChannelCredentials(), args, move(factories));
}

} // namespace grpc_ext
