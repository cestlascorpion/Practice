#pragma once

#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/support/status.h>

#include <boost/thread/shared_mutex.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace grpc_ext {

class grpc_client {
public:
    grpc_client(std::string target, std::string service, std::string authority = "");
    virtual ~grpc_client();

protected:
    void Connect();

    std::shared_ptr<grpc::Channel> Channel();

protected:
    struct callInfo {
        std::string method_;
        uint32_t uin_;
        uint32_t timeout_;

        callInfo()
            : method_()
            , uin_(0)
            , timeout_(3000) {}

        void applyOpts(const std::initializer_list<std::function<void(callInfo &)>> &opts) {
            for (const auto &opt : opts) {
                opt(*this);
            }
        }

        template <typename Head, typename... Tail>
        void applyOpts(Head head, Tail... tail) {
            head(*this);
            applyOpts(tail...);
        }

        void applyOpts() const {
            printf("[%s] method %s uin %u timeout %u do nothing?\n", __func__, method_.c_str(), uin_, timeout_);
        }
    };

protected:
    using UnaryCallFunc = std::function<grpc::Status(grpc::ClientContext *)>;

    template <typename... Opts>
    grpc::Status BlockingUnaryCall(UnaryCallFunc func, Opts... opts) {
        grpc::ClientContext ctx;
        return BlockingUnaryCall(&ctx, func, opts...);
    }

    template <typename... Opts>
    grpc::Status BlockingUnaryCall(grpc::ClientContext *ctx, const UnaryCallFunc &func, Opts... opts) {
        callInfo info;
        info.applyOpts(opts...);
        return BlockingUnaryCall(ctx, func, info);
    }

    grpc::Status BlockingUnaryCall(grpc::ClientContext *context,
                                   const std::function<grpc::Status(grpc::ClientContext *)> &func,
                                   const callInfo &info);

protected:
    static std::function<void(callInfo &)> WithUin(uint32_t uin) {
        return [=](callInfo &info) { info.uin_ = uin; };
    }
    static std::function<void(callInfo &)> WithTimeout(uint32_t milli_seconds) {
        return [=](callInfo &info) { info.timeout_ = milli_seconds; };
    }
    static std::function<void(callInfo &)> WithMethod(const char *method) {
        return [=](callInfo &info) { info.method_ = method; };
    }

private:
    const std::string target_;
    const std::string service_;
    const std::string authority_;
    std::shared_ptr<grpc::Channel> channel_;
    boost::shared_mutex channel_lock_;
    bool egress_enabled_;
};

template <typename Stub>
class StubWrapper {
public:
    explicit StubWrapper(std::string name)
        : name_(std::move(name))
        , last_channel_(nullptr)
        , stub_(nullptr) {
        printf("[%s] construct name %s\n", __func__, name_.c_str());
    }
    ~StubWrapper() {
        printf("[%s] deconstruct name %s\n", __func__, name_.c_str());
    }

public:
    std::shared_ptr<Stub> WithChannel(std::shared_ptr<grpc::Channel> channel) {
        {
            boost::shared_lock<boost::shared_mutex> lk(lock_);
            if (channel == last_channel_) {
                return stub_;
            }
        }

        {
            boost::unique_lock<boost::shared_mutex> lk(lock_);
            if (channel == last_channel_) {
                return stub_;
            }
            printf("[%s] name %s create new stub", __func__, name_.c_str());
            stub_.reset(new Stub(channel));
            last_channel_ = channel;
            return stub_;
        }
    }

private:
    const std::string name_;
    std::shared_ptr<grpc::Channel> last_channel_;
    std::shared_ptr<Stub> stub_;
    boost::shared_mutex lock_;
};

} // namespace grpc_ext

namespace grpc_ext {

int GetUnaryRPCStatusCode(const grpc::ClientContext &ctx);

grpc::Status GetUnaryRPCStatus(const grpc::ClientContext &ctx);

template <typename Request, typename Response, typename Stub, typename Func>
grpc::Status invoke_stub_func(const char *method, grpc::ClientContext *ctx, Stub *stub, Func func,
                              const Request &request, Response *response) {
    printf("[%s] method %s\n", __func__, method);

    auto fn = std::bind(func, stub, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto status = fn(ctx, request, response);
    if (!status.ok()) {
        return status;
    }
    return GetUnaryRPCStatus(*ctx);
}

} // namespace grpc_ext