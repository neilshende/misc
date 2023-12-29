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

// Package main implements a client for Greeter service.
package main

import (
	"context"
	"flag"
	"log"
	"time"
	"strconv"

	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	pb "google.golang.org/grpc/examples/helloworld/helloworld"
)

const (
	defaultName = "world"
)

var (
	addr = flag.String("addr", "localhost:50051", "the address to connect to")
	name = flag.String("name", defaultName, "Name to greet")
)

type async struct {
	err error
	reply *pb.HelloReply
}

func main() {
	flag.Parse()
	// Set up a connection to the server.
	conn, err := grpc.Dial(*addr, grpc.WithTransportCredentials(insecure.NewCredentials()))
	if err != nil {
		log.Fatalf("did not connect: %v", err)
	}
	defer conn.Close()
	c := pb.NewGreeterClient(conn)

	// Contact the server and print out its response.
	ctx, cancel := context.WithTimeout(context.Background(), 60*time.Second)
	defer cancel()
	r, err := c.SayHello(ctx, &pb.HelloRequest{Name: *name})
	if err != nil {
		log.Printf("could not greet: %v", err)
	} else {
		log.Printf("Greeting: %s", r.GetMessage())
	}

	achan := make(chan async, 100)
for i:=1; i<101; i++ {
	go func (myi int) {
		ctx, cancel := context.WithTimeout(context.Background(), 60*time.Second)
		defer cancel()
		r, e := c.SayHello(ctx, &pb.HelloRequest{Name: "Async"+strconv.Itoa(myi)})
		achan <- async{err:e, reply:r}
	} (i)
}
b := 0
for {
	a := <-achan
	if a.err != nil {
		log.Printf("Async call fail: %v", a.err)
	} else {
		log.Printf("Greeting: %s", a.reply.GetMessage())
	}
	b++
	if b>=100 {
		log.Printf("received 100")
		break
	}
}


	ctx2, cancel2 := context.WithTimeout(context.Background(), 60*time.Second)
	defer cancel2()
	r, err = c.SayHelloAgain(ctx2, &pb.HelloRequest{Name: "Fail"})
	if err != nil {
		log.Printf("could not greet: %v", err)
	} else {
		log.Printf("Greeting: %s", r.GetMessage())
	}

  ctx3, cancel3 := context.WithTimeout(context.Background(), 60*time.Second)
  defer cancel3()
  // Make the asynchronous RPC call
  stream, err := c.SayHelloStream(ctx3)
  if err != nil {
    log.Fatalf("failed to call SayHello: %v", err)
  }

  // Send the request asynchronously
  go func() {
    err := stream.Send(&pb.HelloRequest{Name: "Alice"})
    if err != nil {
      log.Fatalf("failed to send request: %v", err)
    }
  }()

  // Receive the response with a timeout
  resp, err := stream.Recv()
  if err != nil {
    if ctx3.Err() == context.DeadlineExceeded {
      log.Println("RPC call timed out")
    } else {
      log.Printf("failed to receive response: %v", err)
    }
  } else {
    log.Printf("Greeting: %s", resp.Message)
  }
}
