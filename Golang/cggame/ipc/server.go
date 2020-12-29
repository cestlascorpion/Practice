package ipc

import (
	"encoding/json"
	"fmt"
)

// Request ...
type Request struct {
	Method string `json:"method"`
	Params string `json:"params"`
}

// Response ...
type Response struct {
	Code string `json:"code"`
	Body string `json:"body"`
}

// Server ...
type Server interface {
	Name() string
	Handle(method, params string) *Response
}

// MyServer ...
type MyServer struct {
	Server
}

// NewMyServer ...
func NewMyServer(server Server) *MyServer {
	return &MyServer{server}
}

// Connect ...
func (s *MyServer) Connect() chan string {
	session := make(chan string, 0)
	go func(c chan string) {
		for {
			request := <-c
			if request == "CLOSE" {
				break
			}
			// req := Request{"foo", "from client"}
			var req Request
			err := json.Unmarshal([]byte(request), &req)
			if err != nil {
				fmt.Println("invalid request")
				return
			}
			fmt.Println("Server Call req:", request, "Unmarshal", req)
			response := s.Handle(req.Method, req.Params)
			resp, err := json.Marshal(response)
			if err != nil {
				fmt.Println("invalid response")
				return
			}
			fmt.Println("Server Call resp:", response, "Marshal", string(resp))
			c <- string(resp)
		}
	}(session)

	fmt.Println("a new session created")
	return session
}
