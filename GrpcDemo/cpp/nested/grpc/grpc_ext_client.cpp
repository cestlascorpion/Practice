#include "grpc_ext_client.h"
#include "grpc_ext_base.h"

#include <exception>
#include <map>
#include <utility>

using namespace std;
using namespace chrono;

using namespace grpc;

namespace grpc_ext {

grpc_client::grpc_client(string target)
    : target_(move(target)) {
    printf("[%s] construct\n", __func__);
    Connect();
}

grpc_client::~grpc_client() {
    printf("[%s] deconstruct\n", __func__);
}

void grpc_client::Connect() {
    printf("[%s] try connect\n", __func__);

    auto channel = CreateInsecureChannel(target_);

    {
        boost::unique_lock<boost::shared_mutex> lk(channel_lock_);
        channel_ = channel;
        printf("[%s] target %s\n", __func__, target_.c_str());
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

    static const auto unary_call_timeout = milliseconds(3000);

    auto timeout = info.timeout_ > 0 ? milliseconds(info.timeout_) : unary_call_timeout;
    context->set_deadline(system_clock::now() + timeout);

    Status status;
    try {
        status = func(context);
    } catch (const exception &ex) {
        printf("[%s] exception %s method %s\n", __func__, ex.what(), info.method_.c_str());
        status = Status(StatusCode(-2), ex.what());
    } catch (...) {
        printf("[%s] unknown exception method %s\n", __func__, info.method_.c_str());
        status = Status(StatusCode(-2), "");
    }

    if (status.error_code() != 0) {
        Connect();
    }

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

Status GetUnaryRPCStatus(const ClientContext &ctx) {
    const auto &header = ctx.GetServerInitialMetadata();
    auto val = metadata_get_first(header, k_server_error_code);
    int code = 0;
    if (!val.empty()) {
        code = (int)strtol(val.c_str(), nullptr, 10);
    }
    auto msg = metadata_get_first(header, k_server_error_message);
    return Status((StatusCode)code, msg);
}

} // namespace grpc_ext