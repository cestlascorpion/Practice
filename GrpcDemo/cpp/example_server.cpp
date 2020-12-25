#include <iostream>
#include <memory>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "example.grpc.pb.h"
#include "example.pb.h"

using namespace std;

namespace example {

class EchoServiceImpl final : public EchoService::Service {
public:
    ::grpc::Status Echo(::grpc::ServerContext *context, const ::example::EchoRequest *request,
                        ::example::EchoResponse *response) override {
        const auto &uid = request->uid();
        const auto &content = request->content();
        printf("peer: %s request: uid %u content %s\n", context->peer().c_str(), uid, content.c_str());
        response->set_uid(uid + 1u);
        response->set_content("[" + to_string(uid) + "]" + content);
        printf("[%s] response %s\n", __func__, response->ShortDebugString().c_str());
        return ::grpc::Status::OK;
    }
};

void RunServer() {
    string address("0.0.0.0:16001");
    EchoServiceImpl service;

    ::grpc::EnableDefaultHealthCheckService(true);
    ::grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort(address, ::grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    unique_ptr<::grpc::Server> server(builder.BuildAndStart());
    cout << "Server listen on " << address << endl;
    server->Wait();
}

} // namespace example

int main() {
    cout << "hello, World!" << endl;
    example::RunServer();
    return 0;
}