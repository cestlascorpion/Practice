go env -w  GO111MODULE=on
go mod tidy
go mod download
go mod vendor
mkdir -p proto
protoc -I=../../proto --go_out=plugins=grpc:proto echo.proto
go build -o build/echo_server server/main.go
go build -o build/echo_client client/main.go