#include "grpc_ext_client.h"
#include "create_channel.h"
#include "grpc_ext_common.h"

#include <exception>
#include <map>
#include <utility>

using namespace std;
using namespace chrono;

using namespace grpc;

namespace grpc_ext {

grpc_client::grpc_client(string target, string service, string authority)
    : target_(move(target))
    , service_(move(service))
    , authority_(move(authority))
    , egress_enabled_(false) {
    printf("[%s] construct\n", __func__);
    Connect();
}

grpc_client::~grpc_client() {
    printf("[%s] deconstruct\n", __func__);
}

void grpc_client::Connect() {
    printf("[%s] try connect\n", __func__);

    bool egress_enabled = false;
    auto channel = CreateInsecureChannel(target_, &egress_enabled);

    {
        boost::unique_lock<boost::shared_mutex> lk(channel_lock_);
        channel_ = channel;
        egress_enabled_ = egress_enabled;
        printf("[%s] target %s egress %d\n", __func__, target_.c_str(), egress_enabled_);
    }
}

shared_ptr<Channel> grpc_client::Channel() {
    boost::shared_lock<boost::shared_mutex> lk(channel_lock_);
    auto channel = channel_;
    return channel;
}

Status grpc_client::BlockingUnaryCall(ClientContext *context, const function<Status(ClientContext *)> &func,
                                      const grpc_client::callInfo &info) {
    printf("[%s]\n", __func__);

    if (!authority_.empty()) {
        context->set_authority(authority_);
    }

    static const string uin_key = "uin";
    static const auto unary_call_timeout = milliseconds(3000);

    auto timeout = info.timeout_ > 0 ? milliseconds(info.timeout_) : unary_call_timeout;
    context->set_deadline(system_clock::now() + timeout);
    context->AddMetadata(uin_key, to_string(info.uin_));

    Status status;
    const auto start = high_resolution_clock::now();
    try {
        status = func(context);
    } catch (const exception &ex) {
        printf("[%s] exception %s service %s method %s\n", __func__, ex.what(), service_.c_str(), info.method_.c_str());
        status = Status(StatusCode(-2), ex.what());
    } catch (...) {
        printf("[%s] unknown exception service %s method %s\n", __func__, service_.c_str(), info.method_.c_str());
        status = Status(StatusCode(-2), "");
    }

    const auto end = chrono::high_resolution_clock::now();

    int status_code = 0, custom_code = 0;
    if (status.error_code() != 0) {
        if (int(status.error_code()) > -100) {
            if (int(status_code) > 0) {
                status_code = -2;
                status = Status(StatusCode(status_code), "");
            } else {
                status_code = status.error_code();
            }
            printf("[%s] grpc error %d %d service %s method %s\n", __func__, status.error_code(), status_code,
                   service_.c_str(), info.method_.c_str());
            Connect();
        } else {
            custom_code = int(status.error_code());
        }
    }

    const auto duration = duration_cast<milliseconds>(end - start).count();
    printf("service %s function %s uin %d cost %ld ms ip %s(%s) status %d result %d\n", service_.c_str(),
           info.method_.c_str(), info.uin_, duration, context->peer().c_str(), context->debug_error_string().c_str(),
           status_code, custom_code);

    return status;
}

} // namespace grpc_ext

namespace grpc_ext {

static string metadata_get_first(const multimap<string_ref, string_ref> &md, const string_ref &key) {
    auto range = md.equal_range(key);
    if (range.first != md.end()) {
        return string(range.first->second.begin(), range.first->second.end());
    }
    return "";
}

int GetUnaryRPCStatusCode(const ClientContext &ctx) {
    const auto &header = ctx.GetServerInitialMetadata();
    auto val = metadata_get_first(header, k_server_error_code_compatible);
    if (!val.empty()) {
        return (int)strtol(val.c_str(), nullptr, 10);
    }
    return 0;
}

Status GetUnaryRPCStatus(const ClientContext &ctx) {
    const auto &header = ctx.GetServerInitialMetadata();
    auto val = metadata_get_first(header, k_server_error_code_compatible);
    int code = 0;
    if (!val.empty()) {
        code = (int)strtol(val.c_str(), nullptr, 10);
    }
    auto msg = metadata_get_first(header, k_server_error_message_compatible);
    return Status((StatusCode)code, msg);
}

} // namespace grpc_ext