#pragma once

#include <grpcpp/support/client_interceptor.h>

namespace grpc_ext {

/*
A factory interface for creation of client interceptors. A vector of
factories can be provided at channel creation which will be used to create a
new vector of client interceptors per RPC. Client interceptor authors should
create a subclass of ClientInterceptorFactorInterface which creates objects
of their interceptors.

 class ClientInterceptorFactoryInterface {
public:
    virtual ~ClientInterceptorFactoryInterface() {}
    // Returns a pointer to an Interceptor object on successful creation, nullptr
    // otherwise. If nullptr is returned, this server interceptor factory is
    // ignored for the purposes of that RPC.
    virtual Interceptor* CreateClientInterceptor(ClientRpcInfo* info) = 0;
};
*/

class UnaryInterceptorFactory : public grpc::experimental::ClientInterceptorFactoryInterface {
public:
    grpc::experimental::Interceptor *CreateClientInterceptor(grpc::experimental::ClientRpcInfo *info) override;
};

} // namespace grpc_ext
