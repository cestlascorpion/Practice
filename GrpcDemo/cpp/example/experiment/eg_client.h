#pragma once

#include <memory>
#include <string>

#include "example.grpc.pb.h"

namespace example {

class Client final {
public:
    explicit Client(const std::string &target);
    ~Client();

public:
    grpc::Status Echo(uint32_t inId, const std::string &request, uint32_t &outId, std::string &response);

private:
    std::unique_ptr<EchoService::Stub> stub_;
};

std::shared_ptr<Client> default_client();

} // namespace example
