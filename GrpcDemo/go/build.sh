go mod tidy
go mod download
go mod vendor
go build -o build/example_server example_server.go
go build -o build/example_client example_client.go