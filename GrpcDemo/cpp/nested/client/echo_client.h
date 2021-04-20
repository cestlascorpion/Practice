#pragma once

#include <memory>
#include <string>

#include "grpc_ext_client.h"
#include "nested_echo.grpc.pb.h"

namespace example {
namespace echo {

class Client_v1 final : private grpc_ext::grpc_client {
public:
    explicit Client_v1(const std::string &target);
    ~Client_v1() override;

public:
    int Echo(uint32_t inId, const std::string &request, uint32_t &outId, std::string &response);

private:
    grpc_ext::StubWrapper<EchoService::Stub> stub_;
};

std::shared_ptr<Client_v1> default_client_v1();

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

std::shared_ptr<Client_v2> default_client_v2();

} // namespace echo
} // namespace example