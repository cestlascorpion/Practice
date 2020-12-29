package ipc

import (
	"testing"
)

type EchoServer struct {
}

func (s *EchoServer) Name() string {
	return "EchoServer"
}

func (s *EchoServer) Handle(method, params string) *Response {
	return &Response{"OK", method + " - " + params}
}

func TestIpc(t *testing.T) {
	server := NewMyServer(&EchoServer{})
	client := NewMyClient(server)

	resp, _ := client.Call("foo", "from client")
	if resp.Code != "OK" || resp.Body != "foo - from client" {
		t.Error("client call failed. resp", resp.Code, resp.Body)
	}
	client.Close()
}
