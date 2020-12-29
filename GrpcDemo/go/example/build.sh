go mod tidy
go mod download
go mod vendor
protoc -I=../../proto --go_out=plugins=grpc:proto example.proto
go build -o build/example_server server/main.go
go build -o build/example_client client/main.go