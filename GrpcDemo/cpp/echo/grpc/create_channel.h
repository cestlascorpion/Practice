#pragma once

#include <grpcpp/channel.h>
#include <memory>
#include <string>

namespace grpc_ext {

std::shared_ptr<grpc::Channel> CreateInsecureChannel(const std::string &target, bool *egress_enabled_ptr = nullptr);

}