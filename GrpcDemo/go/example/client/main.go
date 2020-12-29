package main

import (
	"context"
	"fmt"
	"log"
	"time"

	pb "example/proto"

	"google.golang.org/grpc"
)

const (
	address = "0.0.0.0:16001"
)

func main() {
	fmt.Println("gRPC over Go")
	conn, err := grpc.Dial(address, grpc.WithInsecure(), grpc.WithBlock())
	if err != nil {
		log.Fatalf("connect fail %v", err)
	}
	defer conn.Close()
	c := pb.NewEchoServiceClient(conn)
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	r, err := c.Echo(ctx, &pb.EchoRequest{
		Uid:     1234,
		Content: []byte("hello, world"),
	})
	if err != nil {
		log.Fatalf("rpc fail %v", err)
	}
	log.Printf("Received: %v %v", r.GetUid(), r.GetContent())
}
