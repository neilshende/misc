/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// Package main implements a server for Greeter service.
package main

import (
	"os"
	"os/signal"
	"syscall"
	"io"
	"context"
	"flag"
	"fmt"
	"log"
	"net"
	"time"

	"google.golang.org/grpc"
	pb "google.golang.org/grpc/examples/helloworld/helloworld"
)

var (
	port = flag.Int("port", 50051, "The server port")
)

// server is used to implement helloworld.GreeterServer.
type server struct {
	pb.UnimplementedGreeterServer
}

// SayHello implements helloworld.GreeterServer
func (s *server) SayHello(ctx context.Context, in *pb.HelloRequest) (*pb.HelloReply, error) {
	log.Printf("Received: %v", in.GetName())
	time.Sleep(5*time.Second)
	return &pb.HelloReply{Message: "Hello " + in.GetName()}, nil
}

func (s *server) SayHelloAgain(ctx context.Context, in *pb.HelloRequest) (*pb.HelloReply, error) {
	log.Printf("Received: %v", in.GetName())
	time.Sleep(5*time.Second)
	return &pb.HelloReply{Message: "Hello again " + in.GetName()}, nil
}

func (s *server) SayHelloStream(stream pb.Greeter_SayHelloStreamServer) error {
  errchan := make(chan error)
  for {
    // Receive a request
    req, err := stream.Recv()
    if err == io.EOF {
      return nil // Client has closed the stream
    }
    if err != nil {
      return err
    }
    go func(myreq *pb.HelloRequest) {
	// Process the request and send a response
	log.Printf("Received request: %v", myreq.Name)
	resp := pb.HelloReply{Message: "Hello " + myreq.Name + "!"}
	if err := stream.Send(&resp); err != nil {
		errchan <- err
	}
    }(req)
    select {
    case err := <-errchan:
      return err
    default:
    }
  }
}

func main() {
	flag.Parse()
	lis, err := net.Listen("tcp", fmt.Sprintf(":%d", *port))
	if err != nil {
		log.Fatalf("failed to listen: %v", err)
	}
        gracechnl := make(chan bool)
        waitchnl := make(chan bool)
        sigchnl := make(chan os.Signal, 1)
        signal.Notify(sigchnl, syscall.SIGTERM) //we can add more sycall.SIGQUIT etc.
        go func() {
                for {
                        signal := <-sigchnl
                        fmt.Println(time.Now().Format(time.RFC3339), " Signal received:", signal.String())
                        gracechnl <- true
                        <-waitchnl
                        fmt.Println(time.Now().Format(time.RFC3339), " grpc server shutdown complete, exiting")
                        os.Exit(0)
                }
        }()

	s := grpc.NewServer()
	pb.RegisterGreeterServer(s, &server{})

        go func() {
                <-gracechnl
                fmt.Println(time.Now().Format(time.RFC3339), " Initiating graceful shutdown")
                s.GracefulStop()
                waitchnl <- true
        }()


	log.Printf("server listening at %v", lis.Addr())
	if err := s.Serve(lis); err != nil {
		log.Fatalf("failed to serve: %v", err)
	}
}
