package main

import (
    "context"
    "log"
    "time"

    "google.golang.org/grpc"
    pb "grpc-example/greeterpb"
)

func main() {
    conn, err := grpc.Dial("localhost:50051", grpc.WithInsecure(), grpc.WithBlock())
    if err != nil {
        log.Fatalf("Did not connect: %v", err)
    }
    defer conn.Close()
    c := pb.NewGreeterClient(conn)

    ctx, cancel := context.WithTimeout(context.Background(), time.Second)
    defer cancel()

    res, err := c.SayHello(ctx, &pb.HelloRequest{Name: "Go Developer"})
    if err != nil {
        log.Fatalf("Could not greet: %v", err)
    }
    log.Printf("Greeting: %s", res.Message)
}
