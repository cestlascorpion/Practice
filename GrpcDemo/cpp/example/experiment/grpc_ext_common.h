#pragma once

namespace grpc_ext {

constexpr const char *k_request_uid = "x-request-uid";

constexpr const char *k_server_error_code = "x-server-error-code";
constexpr const char *k_server_error_message = "x-server-error-message";

constexpr const char *k_ot_span_context = "x-ot-span-context";

constexpr const char *k_server_error_code_compatible = "resp_code";
constexpr const char *k_server_error_message_compatible = "resp_msg";

} // namespace grpc_ext