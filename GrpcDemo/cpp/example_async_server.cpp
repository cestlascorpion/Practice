#include <iostream>
#include <memory>

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#include "example.grpc.pb.h"
#include "example.pb.h"

using namespace std;

namespace example {

class ExampleServer final {
public:
    ~ExampleServer() {}

public:
    void Run() {
        string address("0.0.0.0:16001");
        ::grpc::ServerBuilder builder;
        builder.AddListeningPort(address, ::grpc::InsecureServerCredentials());
        builder.RegisterService(&service_);
        cq_ = builder.AddCompletionQueue();
        server_ = builder.BuildAndStart();
        cout << "Server listen on " << address << endl;
        HandleRpcs();
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
                service_->RequestEcho(&ctx_, &request_, &responder_, cq_, cq_, this);
            } else if (status_ == PROCESS) {
                new CallData(service_, cq_);
                reply_.set_uid(request_.uid() + 1);
                reply_.set_content("[" + to_string(request_.uid()) + "]" + request_.content());
                status_ = FINISH;
                responder_.Finish(reply_, ::grpc::Status::OK, this);
            } else {
                GPR_ASSERT(status_ == FINISH);
                delete this;
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
    void HandleRpcs() {
        new CallData(&service_, cq_.get());
        void *tag;
        bool ok;
        while (true) {
            GPR_ASSERT(cq_->Next(&tag, &ok));
            GPR_ASSERT(ok);
            static_cast<CallData *>(tag)->Proceed();
        }
    }

private:
    std::unique_ptr<::grpc::ServerCompletionQueue> cq_;
    EchoService::AsyncService service_;
    std::unique_ptr<::grpc::Server> server_;
};

} // namespace example

int main() {
    cout << "hello, World!" << endl;
    example::ExampleServer server;
    server.Run();
    return 0;
}