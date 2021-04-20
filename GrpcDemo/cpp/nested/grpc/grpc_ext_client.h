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
    explicit grpc_client(std::string target);
    virtual ~grpc_client();

protected:
    void Connect();

    std::shared_ptr<grpc::Channel> Channel();

protected:
    struct callInfo {
        std::string method_;
        uint32_t timeout_;

        explicit callInfo(std::string method)
            : method_(std::move(method))
            , timeout_(3000) {}
    };

protected:
    grpc::Status BlockingUnaryCall(grpc::ClientContext *context,
                                   const std::function<grpc::Status(grpc::ClientContext *)> &func,
                                   const callInfo &info);

protected:
    static std::function<void(callInfo &)> WithMethod(const char *method) {
        return [=](callInfo &info) { info.method_ = method; };
    }

private:
    const std::string target_;
    std::shared_ptr<grpc::Channel> channel_;
    boost::shared_mutex channel_lock_;
};

template <typename Stub>
class StubWrapper {
public:
    explicit StubWrapper()
        : channel_(nullptr)
        , stub_(nullptr) {
        printf("[%s] construct\n", __func__);
    }
    ~StubWrapper() {
        printf("[%s] deconstruct\n", __func__);
    }

public:
    std::shared_ptr<Stub> WithChannel(std::shared_ptr<grpc::Channel> channel) {
        {
            boost::shared_lock<boost::shared_mutex> lk(lock_);
            if (channel == channel_) {
                return stub_;
            }
        }

        {
            boost::unique_lock<boost::shared_mutex> lk(lock_);
            if (channel == channel_) {
                return stub_;
            }
            printf("[%s] create new stub\n", __func__);
            stub_.reset(new Stub(channel));
            channel_ = channel;
            return stub_;
        }
    }

private:
    std::shared_ptr<grpc::Channel> channel_;
    std::shared_ptr<Stub> stub_;
    boost::shared_mutex lock_;
};

} // namespace grpc_ext

namespace grpc_ext {

grpc::Status GetUnaryRPCStatus(const grpc::ClientContext &ctx);

template <typename Req, typename Resp, typename Stub, typename Func>
grpc::Status invoke_stub_func(grpc::ClientContext *ctx, Stub *stub, Func func, const Req &req, Resp *resp) {
    auto fn = std::bind(func, stub, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    auto status = fn(ctx, req, resp);
    if (!status.ok()) {
        return status;
    }
    return GetUnaryRPCStatus(*ctx);
}

} // namespace grpc_ext