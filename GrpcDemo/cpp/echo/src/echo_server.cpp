#include <memory>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "echo.grpc.pb.h"
#include "echo.pb.h"

using namespace std;

namespace echo {

// 继承并重写服务要求的方法
class EchoServiceImpl final : public EchoService::Service {
public:
    ::grpc::Status Echo(::grpc::ServerContext *context, const ::echo::EchoRequest *request,
                        ::echo::EchoResponse *response) override {
        // 检查超时
        if (chrono::system_clock::now() < context->deadline()) {
            printf("request %s\n", request->ShortDebugString().c_str());
            response->set_uid(request->uid());
            response->set_content(request->content());
            printf("response %s\n", response->ShortDebugString().c_str());
            return ::grpc::Status::OK;
        }
        ::grpc::Status status(::grpc::StatusCode::DEADLINE_EXCEEDED, "timeout");
        return status;
    }
};

void RunServer() {
    string address("0.0.0.0:16001");
    // 创建 server 对象并设置属性
    EchoServiceImpl service;
    ::grpc::EnableDefaultHealthCheckService(true);
    ::grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    ::grpc::ServerBuilder builder;
    builder.AddListeningPort(address, ::grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    unique_ptr<::grpc::Server> server(builder.BuildAndStart());
    printf("Server listen on %s\n", address.c_str());
    server->Wait();
}

} // namespace echo

int main() {
    echo::RunServer();
    return 0;
}