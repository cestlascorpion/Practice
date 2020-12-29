package ipc

import (
	"encoding/json"
	"fmt"
)

// MyClient ...
type MyClient struct {
	conn chan string
}

// NewMyClient ...
func NewMyClient(server *MyServer) *MyClient {
	c := server.Connect()
	return &MyClient{c}
}

// Call ...
func (c *MyClient) Call(method, params string) (*Response, error) {
	req := &Request{method, params}
	request, err1 := json.Marshal(req)
	if err1 != nil {
		fmt.Println("invalid request")
		return nil, err1
	}
	// fmt.Println("Client Call req:", req, "Marshal", string(request))
	c.conn <- string(request)
	response := <-c.conn

	var resp Response
	err2 := json.Unmarshal([]byte(response), &resp)
	if err2 != nil {
		fmt.Println("invalid response")
		return nil, err2
	}
	// fmt.Println("Client Call resp:", string(response), "Unmarshal", resp)
	return &resp, nil
}

// Close ...
func (c *MyClient) Close() {
	c.conn <- "CLOSE"
}
