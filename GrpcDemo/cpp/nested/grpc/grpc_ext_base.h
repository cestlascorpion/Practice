#pragma once

#include <grpcpp/channel.h>
#include <grpcpp/support/client_interceptor.h>

#include <memory>
#include <string>

namespace grpc_ext {

constexpr const char *k_ot_span_context = "x-ot-span-context";
constexpr const char *k_server_error_code = "x-server-error-code";
constexpr const char *k_server_error_message = "x-server-error-message";

} // namespace grpc_ext

namespace grpc_ext {

class UnaryInterceptorFactory : public grpc::experimental::ClientInterceptorFactoryInterface {
public:
    grpc::experimental::Interceptor *CreateClientInterceptor(grpc::experimental::ClientRpcInfo *info) override;
};

} // namespace grpc_ext

namespace grpc_ext {

std::shared_ptr<grpc::Channel> CreateInsecureChannel(const std::string &target);

}