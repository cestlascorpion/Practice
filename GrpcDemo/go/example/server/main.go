package main

import (
	"context"
	"fmt"
	"log"
	"net"

	pb "example/proto"

	"google.golang.org/grpc"
)

const (
	port = ":16001"
)

type server struct {
	pb.UnimplementedEchoServiceServer
}

func (s *server) Echo(ctx context.Context, in *pb.EchoRequest) (*pb.EchoResponse, error) {
	log.Printf("Received: %v %v", in.GetUid(), in.GetContent())
	return &pb.EchoResponse{
		Uid:     in.GetUid() + 1,
		Content: in.GetContent(),
	}, nil
}

func main() {
	fmt.Println("gRPC over Go")
	lis, err := net.Listen("tcp", port)
	if err != nil {
		log.Fatalf("fail to listen %v", err)
	}
	s := grpc.NewServer()
	pb.RegisterEchoServiceServer(s, &server{})
	if err := s.Serve(lis); err != nil {
		log.Fatalf("fail to server %v", err)
	}
}
