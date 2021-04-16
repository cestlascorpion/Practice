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

static const char *k_server_error_code_compatible = "resp_code";
static const char *k_server_error_message_compatible = "resp_msg";

static bool meta_get(const std::multimap<grpc::string_ref, grpc::string_ref> &meta, const grpc::string_ref &key,
                     grpc::string *value) {
    auto range = meta.equal_range(key);
    if (range.first != meta.end()) {
        value->assign(range.first->second.begin(), range.first->second.end());
        return true;
    }
    return false;
}

static bool meta_get_int(const std::multimap<grpc::string_ref, grpc::string_ref> &meta, const grpc::string_ref &key,
                         int *value) {
    grpc::string val;
    if (meta_get(meta, key, &val)) {
        *value = (int)strtol(val.c_str(), nullptr, 10);
        return true;
    }
    return false;
}

/*
Interface for an interceptor. Interceptor authors must create a class
that derives from this parent class.

class Interceptor {
public:
    virtual ~Interceptor() {}

    /// The one public method of an Interceptor interface. Override this to
    /// trigger the desired actions at the hook points described above.
    virtual void Intercept(InterceptorBatchMethods* methods) = 0;
};
*/

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
        // other: CLIENT_STREAMING, SERVER_STREAMING, BIDI_STREAMING,
        if (info_->type() != ClientRpcInfo::Type::UNARY) {
            return;
        }
        /*
        enum class InterceptionHookPoints {
            /// The first three in this list are for clients and servers
            PRE_SEND_INITIAL_METADATA,
            PRE_SEND_MESSAGE,
            POST_SEND_MESSAGE,
            PRE_SEND_STATUS,  // server only
            PRE_SEND_CLOSE,   // client only: WritesDone for stream; after write in unary
            /// The following three are for hijacked clients only. A batch with PRE_RECV_*
            /// hook points will never contain hook points of other types.
            PRE_RECV_INITIAL_METADATA,
            PRE_RECV_MESSAGE,
            PRE_RECV_STATUS,
            /// The following two are for all clients and servers
            POST_RECV_INITIAL_METADATA,
            POST_RECV_MESSAGE,
            POST_RECV_STATUS,  // client only
            POST_RECV_CLOSE,   // server only
            /// This is a special hook point available to both clients and servers when
            /// TryCancel() is performed.
            ///  - No other hook points will be present along with this.
            ///  - It is illegal for an interceptor to block/delay this operation.
            ///  - ALL interceptors see this hook point irrespective of whether the
            ///    RPC was hijacked or not.
            PRE_SEND_CANCEL,
            NUM_INTERCEPTION_HOOKS
        };
        */
        // Determine whether the current batch has an interception hook point
        // of type \a type
        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::PRE_SEND_INITIAL_METADATA)) {
            printf("hook point PRE_SEND_INITIAL_METADATA\n");
            start_ = high_resolution_clock::now();
            auto meta = methods->GetSendInitialMetadata();
            auto iter = meta->find(k_request_uid);
            if (iter != meta->end()) {
                uid_ = (uint32_t)stoul(iter->second, nullptr, 10);
            }
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
            auto status = methods->GetRecvStatus();
            int status_code = 0, custom_code = 0;
            grpc::string status_message, custom_message;
            if (status != nullptr) {
                status_code = status->error_code();
                status_message = status->error_message();
                if (status->ok()) {
                    retrieveStatusFromMetadata(*methods->GetRecvInitialMetadata(), &custom_code, &custom_message);
                }
            } else {
                status_code = -2;
                status_message = "invalid";
            }
            auto duration = duration_cast<milliseconds>(high_resolution_clock::now() - start_).count();
            auto SvcAndFunc = getServiceAndFunction(info_->method());
            auto channelState = info_->channel()->GetState(false);
            printf("service %s function %s cost %ld ms ip %s(%s) status %d(%s) result %d(%s) conn %d",
                   get<0>(SvcAndFunc).c_str(), get<1>(SvcAndFunc).c_str(), duration,
                   info_->client_context()->peer().c_str(), info_->client_context()->debug_error_string().c_str(),
                   status_code, status_message.c_str(), custom_code, custom_message.c_str(), channelState);
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
        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::POST_RECV_CLOSE)) {
            printf("hook point POST_RECV_CLOSE\n");
        }
        if (methods->QueryInterceptionHookPoint(InterceptionHookPoints::PRE_SEND_CANCEL)) {
            printf("hook point PRE_SEND_CANCEL\n");
        }
        methods->Proceed();
    }

private:
    static void retrieveStatusFromMetadata(const multimap<string_ref, string_ref> &meta, int *code,
                                           grpc::string *message) {
        if (meta_get_int(meta, k_server_error_code, code)) {
            meta_get(meta, k_server_error_message, message);
            return;
        }

        if (meta_get_int(meta, k_server_error_code_compatible, code)) {
            meta_get(meta, k_server_error_message_compatible, message);
            return;
        }
        printf("no custom code and message\n");
    }

    static tuple<grpc::string, grpc::string> getServiceAndFunction(const char *method) {
        {
            boost::shared_lock<boost::shared_mutex> lk(known_methods_lock_);
            auto iter = known_methods_.find(method);
            if (iter != known_methods_.end()) {
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
            known_methods_.emplace(method, tuple);
        }

        return tuple;
    }

private:
    static unordered_map<const char *, tuple<grpc::string, grpc::string>> known_methods_;
    static boost::shared_mutex known_methods_lock_;

private:
    ClientRpcInfo *info_;
    uint32_t uid_;
    high_resolution_clock::time_point start_;
};

unordered_map<const char *, tuple<grpc::string, grpc::string>> UnaryInterceptor::known_methods_;
boost::shared_mutex UnaryInterceptor::known_methods_lock_;

namespace grpc_ext {

Interceptor *UnaryInterceptorFactory::CreateClientInterceptor(ClientRpcInfo *info) {
    return new UnaryInterceptor(info);
}

} // namespace grpc_ext
