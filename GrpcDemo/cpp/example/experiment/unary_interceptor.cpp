#include "unary_interceptor.h"

#include <grpc/support/log.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/impl/codegen/string_ref.h>

#include <boost/thread/shared_mutex.hpp>

#include <map>
#include <string>
#include <unordered_map>

using namespace std;
using namespace chrono;

using namespace grpc;
using namespace experimental;

static const char *k_request_uid = "x-request-uid";

static const char *k_server_error_code = "x-server-error-code";
static const char *k_server_error_message = "x-server-error-message";

static const char *k_ot_span_context = "x-ot-span-context";

static const char *k_server_error_code_compatible = "resp_code";
static const char *k_server_error_message_compatible = "resp_msg";

using MetaType = multimap<grpc::string_ref, grpc::string_ref>;

static bool MetaGet(const MetaType &meta, const grpc::string_ref &key, grpc::string *value) {
    auto range = meta.equal_range(key);
    if (range.first != meta.end()) {
        value->assign(range.first->second.begin(), range.first->second.end());
        return true;
    }
    return false;
}

static bool MetaGet(const MetaType &meta, const grpc::string_ref &key, int *value) {
    grpc::string val;
    if (MetaGet(meta, key, &val)) {
        *value = (int)strtol(val.c_str(), nullptr, 10);
        return true;
    }
    return false;
}

class UnaryInterceptor : public Interceptor {
public:
    explicit UnaryInterceptor(ClientRpcInfo *info)
        : info_(info)
        , uid_(0)
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
            auto iter = meta->find(k_request_uid);
            if (iter != meta->end()) {
                uid_ = (uint32_t)stoul(iter->second, nullptr, 10);
                printf("[%s] uid %u", __func__, uid_);
            }
            meta->emplace(k_ot_span_context, "trace id for test");
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
            grpc::string status_message, custom_message;
            if (status != nullptr) {
                status_code = status->error_code();
                status_message = status->error_message();
                if (status->ok()) {
                    InvokeMeta(*methods->GetRecvInitialMetadata(), &custom_code, &custom_message);
                }
            } else {
                status_code = -2;
                status_message = "invalid";
            }
            auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start_).count();
            auto SvcAndFunc = GetMethod(info_->method());
            auto channelState = info_->channel()->GetState(false);
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
    static void InvokeMeta(const MetaType &meta, int *code, grpc::string *message) {
        if (MetaGet(meta, k_server_error_code, code)) {
            MetaGet(meta, k_server_error_message, message);
            return;
        }

        if (MetaGet(meta, k_server_error_code_compatible, code)) {
            MetaGet(meta, k_server_error_message_compatible, message);
            return;
        }

        printf("no custom code and message\n");
    }

    static tuple<grpc::string, grpc::string> GetMethod(const char *method) {
        {
            boost::shared_lock<boost::shared_mutex> lk(methods_lock_);
            auto iter = methods_.find(method);
            if (iter != methods_.end()) {
                printf("find method %s/%s\n", get<0>(iter->second).c_str(), get<1>(iter->second).c_str());
                return iter->second;
            }
        }

        const std::string extra(method);
        printf("add method %s\n", extra.c_str());
        auto first_slash = extra.find_first_of('/', 1);
        auto service = extra.substr(1, first_slash - 1);
        auto function = extra.substr(first_slash + 1);
        auto tuple = make_tuple(service, function);

        {
            boost::unique_lock<boost::shared_mutex> lk;
            methods_.emplace(method, tuple);
        }

        return tuple;
    }

private:
    static unordered_map<const char *, tuple<grpc::string, grpc::string>> methods_;
    static boost::shared_mutex methods_lock_;

private:
    ClientRpcInfo *info_;
    uint32_t uid_;
    high_resolution_clock::time_point start_;
};

unordered_map<const char *, tuple<grpc::string, grpc::string>> UnaryInterceptor::methods_;
boost::shared_mutex UnaryInterceptor::methods_lock_;

namespace grpc_ext {

Interceptor *UnaryInterceptorFactory::CreateClientInterceptor(ClientRpcInfo *info) {
    return new UnaryInterceptor(info);
}

} // namespace grpc_ext
