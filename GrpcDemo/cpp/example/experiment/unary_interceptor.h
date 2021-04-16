#pragma once

#include <grpcpp/support/client_interceptor.h>

namespace grpc_ext {

class UnaryInterceptorFactory : public grpc::experimental::ClientInterceptorFactoryInterface {
public:
    grpc::experimental::Interceptor *CreateClientInterceptor(grpc::experimental::ClientRpcInfo *info) override;
};

} // namespace grpc_ext
