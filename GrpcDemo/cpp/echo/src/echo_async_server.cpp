#include <memory>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include "echo.grpc.pb.h"
#include "echo.pb.h"

using namespace std;

namespace echo {

class ExampleServer final {
public:
    void Run() {
        string address("0.0.0.0:16001");
        ::grpc::ServerBuilder builder;
        builder.AddListeningPort(address, ::grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        printf("Server listen on %s\n", address.c_str());

        new CallData(&service_, cq_.get()); // 构造时会把 calldata 塞进 cq
        void *tag;
        bool ok;
        // 使用异步方法做同步处理 但是不会阻塞在网络上
        while (true) {
            GPR_ASSERT(cq_->Next(&tag, &ok));
            GPR_ASSERT(ok);
            static_cast<CallData *>(tag)->Proceed();
        }
    }

private:
    class CallData {
    public:
        CallData(EchoService::AsyncService *service, ::grpc::ServerCompletionQueue *cq)
            : service_(service)
            , cq_(cq)
            , responder_(&ctx_)
            , status_(CREATE) {
            Proceed();
        }

    public:
        void Proceed() {
            if (status_ == CREATE) {
                status_ = PROCESS;
                service_->RequestEcho(&ctx_, &request_, &responder_, cq_, cq_, this); // 使用了同一个 cq ？
            } else if (status_ == PROCESS) {
                new CallData(service_, cq_); // 塞一个新的 calldata
                reply_.set_uid(request_.uid());
                reply_.set_content(request_.content());
                status_ = FINISH;
                responder_.Finish(reply_, ::grpc::Status::OK, this);
            } else {
                GPR_ASSERT(status_ == FINISH);
                delete this; // 删除当前的 calldata
            }
        }

    private:
        EchoService::AsyncService *service_;
        ::grpc::ServerCompletionQueue *cq_;
        ::grpc::ServerContext ctx_;
        EchoRequest request_;
        EchoResponse reply_;
        ::grpc::ServerAsyncResponseWriter<EchoResponse> responder_;

        enum CallStatus { CREATE, PROCESS, FINISH };
        CallStatus status_;
    };

private:
    std::unique_ptr<::grpc::ServerCompletionQueue> cq_;
    EchoService::AsyncService service_;
    std::unique_ptr<::grpc::Server> server_;
};

} // namespace echo

int main() {
    echo::ExampleServer server;
    server.Run();
    return 0;
}