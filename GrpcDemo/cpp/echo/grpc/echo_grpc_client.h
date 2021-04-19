#pragma once

#include <memory>
#include <string>

#include "echo.grpc.pb.h"
#include "grpc_ext_client.h"

namespace echo {

class Client_v1 final : private grpc_ext::grpc_client {
public:
    Client_v1();
    ~Client_v1() override;

public:
    int Echo(uint32_t inId, const std::string &request, uint32_t &outId, std::string &response);

private:
    grpc_ext::StubWrapper<EchoService::Stub> stub_;
};

std::shared_ptr<Client_v1> shared_client();

} // namespace echo

namespace echo {

class Client_v2 final {
public:
    explicit Client_v2(const std::string &target);
    ~Client_v2();

public:
    grpc::Status Echo(uint32_t inId, const std::string &request, uint32_t &outId, std::string &response);

private:
    std::unique_ptr<EchoService::Stub> stub_;
};

std::shared_ptr<Client_v2> default_client();

} // namespace echo
