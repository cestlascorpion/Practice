protoc -I=../proto --grpc_out=build --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin example.proto
protoc -I=../proto --cpp_out=build example.proto