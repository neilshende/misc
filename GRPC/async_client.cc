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

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <semaphore>
#include <mutex>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

#include <unistd.h>

ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;
using helloworld::YelloReply;
using helloworld::YelloRequest;

  // struct for keeping state and data information
  struct AsyncClientCall {
    // Container for the data we expect from the server.
    int yello;
       HelloReply hreply;
       YelloReply yreply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // Storage for the status of the RPC upon completion.
    Status status;

    // signal when done
    std::counting_semaphore<1> sem{0};

       std::unique_ptr<ClientAsyncResponseReader<HelloReply>> hresponse_reader;
       std::unique_ptr<ClientAsyncResponseReader<YelloReply>> yresponse_reader;
  };

// This class implements async client for a service. The object created via this class
// shall remain functional as long as the service is up. If the server goes down, delete
// the object and re-create it with server's new address.
class GreeterClient {
 public:
  explicit GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {
  shutdown_ = false;
  // Spawn reader thread that loops indefinitely
  thread_ = std::thread(&GreeterClient::AsyncCompleteRpc, this);

  }
  ~GreeterClient() {
      shutdown_ = true;
      std::cout << "Trying to quit\n";
      cq_.Shutdown();
      thread_.join();
      std::cout << "BYE BYE\n";
  }

  // Assembles the client's payload and sends it to the server.
  AsyncClientCall* SayHello(const std::string& user) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(user);

    // Call object to store rpc data
    AsyncClientCall* call = new AsyncClientCall;
    call->yello = 0;

    // stub_->PrepareAsyncSayHello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    call->hresponse_reader =
        stub_->PrepareAsyncSayHello(&call->context, request, &cq_);

    // StartCall initiates the RPC call
    call->hresponse_reader->StartCall();

    // Request that, upon completion of the RPC, "reply" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the memory address of the call
    // object.
    call->hresponse_reader->Finish(&call->hreply, &call->status, (void*)call);
    return call;
  }

  // Assembles the client's payload and sends it to the server.
  AsyncClientCall* SayYello(const std::string& user) {
    // Data we are sending to the server.
    YelloRequest request;
    request.set_name(user);

    // Call object to store rpc data
    AsyncClientCall* call = new AsyncClientCall;
    call->yello = 1;

    // stub_->PrepareAsyncSayYello() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    call->yresponse_reader =
        stub_->PrepareAsyncSayYello(&call->context, request, &cq_);

    // StartCall initiates the RPC call
    call->yresponse_reader->StartCall();

    // Request that, upon completion of the RPC, "reply" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the memory address of the call
    // object.
    call->yresponse_reader->Finish(&call->yreply, &call->status, (void*)call);
    return call;
  }
  // Loop while listening for completed responses.
  // Prints out the response from the server.
  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    // Block until the next result is available in the completion queue "cq".
    while (cq_.Next(&got_tag, &ok)) {
      // The tag in this example is the memory location of the call object
      AsyncClientCall* call = static_cast<AsyncClientCall*>(got_tag);

      // Verify that the request was completed successfully. Note that "ok"
      // corresponds solely to the request for updates introduced by Finish().
      GPR_ASSERT(ok);

      call->sem.release();
      if (shutdown_) break;
    }
  }

 private:
  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<Greeter::Stub> stub_;

  // The producer-consumer queue we use to communicate asynchronously with the
  // gRPC runtime.
  CompletionQueue cq_;
  std::thread thread_;
  bool shutdown_;
};

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  std::string target_str = absl::GetFlag(FLAGS_target);
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  GreeterClient *greeter = new GreeterClient(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  int N = 100;
  int i;
  AsyncClientCall* call_contexts[N];

  for (i = 0; i < N; i++) {
    std::string user("world " + std::to_string(i));
    if (i%2) {
       std::cout <<  "calling hello " << i << "\n";
       call_contexts[i] = greeter->SayHello(user);  // The actual RPC call!
    } else {
       std::cout <<  "calling yello " << i << "\n";
       call_contexts[i] = greeter->SayYello(user);
    }
  }
  for (i = 0; i < N; i++) {
      call_contexts[i]->sem.acquire(); // We can enforce deadline here by using acquire_for(duration)
      if (call_contexts[i]->status.ok())
        if (call_contexts[i]->yello == 1) {
            std::cout << "Greeter received yello: " << call_contexts[i]->yreply.message() << std::endl;
        } else {
            std::cout << "Greeter received: " << call_contexts[i]->hreply.message() << std::endl;
        }
      else
        std::cout << "RPC failed" << std::endl;
  }
  std::cout << "Quitting\n";
  for (i = 0; i < N; i++) {

      // Once we're complete, deallocate the call object.
      delete call_contexts[i];
  }
  delete greeter;
  return 0;
}
