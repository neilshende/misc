package main

import (
    "context"
    "log"
    "net"

    "google.golang.org/grpc"
    pb "grpc-example/greeterpb"
)

type server struct {
    pb.UnimplementedGreeterServer
}

func (s *server) SayHello(ctx context.Context, req *pb.HelloRequest) (*pb.HelloResponse, error) {
    log.Printf("Received: %s", req.Name)
    return &pb.HelloResponse{Message: "Hello " + req.Name}, nil
}

func main() {
    lis, err := net.Listen("tcp", ":50051")
    if err != nil {
        log.Fatalf("Failed to listen: %v", err)
    }
    grpcServer := grpc.NewServer()
    pb.RegisterGreeterServer(grpcServer, &server{})
    log.Println("Server listening on port 50051")
    if err := grpcServer.Serve(lis); err != nil {
        log.Fatalf("Failed to serve: %v", err)
    }
}
