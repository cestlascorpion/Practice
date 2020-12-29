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

	resp1, _ := client.Call("foo", "from client1")
	resp2, _ := client.Call("foo", "from client2")
	if resp1.Code != "OK" || resp1.Body != "foo - from client1" || resp2.Code != "OK" || resp2.Body != "foo - from client2" {
		t.Error("client call failed. resp", resp1, resp2)
	}
	client.Close()
}
