
rpcid=($(seq 1 ${#rpcs[@]}))
pref=(${rpcs[@]})

deadline_ms=-1
wait_for_ready="true"
max_retry_count=3

sem_bug=1
unit_test=1

cat <<EOF
#define SEM_BUG ${sem_bug}
#define UNIT_TEST ${unit_test}

#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <semaphore>
#include <chrono>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpc/support/log.h>
#include <grpcpp/grpcpp.h>
#ifdef BAZEL_BUILD
#include "examples/protos/${package}.grpc.pb.h"
#else
#include "${package}.grpc.pb.h"
#endif

#include <unistd.h>

using grpc::Channel;
using grpc::ClientAsyncResponseReader;
using grpc::ClientContext;
using grpc::CompletionQueue;
using grpc::Status;
using ${package}::${service};

ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

EOF

for reply in ${replies[@]}; do
cat <<EOF
using ${package}::${reply};
EOF
done

for request in ${requests[@]}; do
cat <<EOF
using ${package}::${request};
EOF
done

cat <<EOF
  // struct for keeping state and data information
  class ${service}AsyncClientCall {
  public:
     ${service}AsyncClientCall() {retry_count = 0;};
     ~${service}AsyncClientCall() { delete context; };

    // how many retries so far?
    int retry_count;

    // identify which call was made
    int rpcid;

    // Container for the data we expect to and from the server.
EOF

i=0
for reply in ${replies[@]}; do
cat <<EOF
    ${requests[$i]} ${pref[$i]}_request;
    ${reply} ${pref[$i]}_reply;

EOF
((i++))
done

cat <<EOF

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext *context;

    // Storage for the status of the RPC upon completion.
    Status status;

    // signal when done
#if SEM_BUG
    std::mutex sem;
#else
    std::counting_semaphore<1> sem{1};
#endif

   // Async responses readers 
EOF

i=0
for reply in ${replies[@]}; do
cat <<EOF
    std::unique_ptr<ClientAsyncResponseReader<${reply}>> ${pref[$i]}_response_reader;
EOF
((i++))
done

cat <<EOF
  };

// This class implements async and sync client for a service. The object created via this class
// shall remain functional as long as the service is up. If the server goes down,
// this class will automatically reconnect. Any RPC that failed with UNAVIALABLE will be
// automatically dispatched. The retried RPC will wait for server to be ready. It is therefore
// necessay to keep the deadline large or not set the deadline for each RPC at all.
class ${service}Client {
 public:
  explicit ${service}Client(std::shared_ptr<Channel> channel,
      int deadline_ms = ${deadline_ms},
      bool wait_for_ready = ${wait_for_ready},
      int max_retry_count = ${max_retry_count})
      : stub_(${service}::NewStub(channel)) {

  shutdown_ = false;
  deadline_ms_ = deadline_ms;
  wait_for_ready_ = wait_for_ready;
  max_retry_count_ = max_retry_count;

  // Spawn reader thread that loops indefinitely
  thread_ = std::thread(&${service}Client::AsyncCompleteRpc, this);

  }
  ~${service}Client() {
      shutdown_ = true;
      cq_.Shutdown();
      thread_.join();
  }
EOF

i=0
for rpc in ${rpcs[@]}; do
cat <<EOF

  //sync rpc ${rpc}
  void ${rpc}(const ${requests[$i]} req,
                ${replies[$i]} *rep,
                int *err) {
     ${service}AsyncClientCall *call = ${rpc}(req);
#if SEM_BUG
     call->sem.lock();
#else
     call->sem.acquire();
#endif
     *err = call->status.error_code();
     if (*err == 0) {
        *rep = call->${pref[$i]}_reply;
     }
     delete call;
   }

  // async as well as retry entry point for ${rpc}
  // Assembles the client's payload and sends it to the server.
  ${service}AsyncClientCall* ${rpc}(const ${requests[$i]} req,
     ${service}AsyncClientCall *callp=NULL) {
    // Data we are sending to the server.
    //${requests[i]} request(req);

    ${service}AsyncClientCall* call = NULL;
    if (callp == NULL) {
        // Call object to store rpc data
        call = new ${service}AsyncClientCall;
        call->rpcid = ${rpcid[$i]};
        call->${pref[i]}_request = req;
#if SEM_BUG
        call->sem.lock();
#else
        call->sem.acquire();
#endif
    } else {
        // This is a retry
        callp->retry_count++;
        std::cout << "retrying: count=" << callp->retry_count << std::endl;
        call = callp;
        delete call->context;
    }
    call->context = new ClientContext;

    if (deadline_ms_ > 0) {
       // Set deadline for this rpc.
       // default deadline for GRPC is very large -- if the server is up
       // it's almost an eternity.
       // If the server is down, the rpc will fail quickly with UNAVAILABLE (14)
       // If the server does not respond before the deadline, the client
       // will error out with DEADLINE_EXCEEDED (4).
       auto deadline = std::chrono::system_clock::now() +
       std::chrono::milliseconds(deadline_ms_);
       call->context->set_deadline(deadline);
    }

    call->context->set_wait_for_ready(wait_for_ready_);

    // stub_->PrepareAsync${rpc}() creates an RPC object, returning
    // an instance to store in "call" but does not actually start the RPC
    // Because we are using the asynchronous API, we need to hold on to
    // the "call" instance in order to get updates on the ongoing RPC.
    call->${pref[$i]}_response_reader =
        stub_->PrepareAsync${rpc}(call->context, call->${pref[i]}_request, &cq_);

    // StartCall initiates the RPC call
    call->${pref[$i]}_response_reader->StartCall();

    // Request that, upon completion of the RPC, "reply" be updated with the
    // server's response; "status" with the indication of whether the operation
    // was successful. Tag the request with the memory address of the call
    // object.
    call->${pref[$i]}_response_reader->Finish(&call->${pref[$i]}_reply, &call->status, (void*)call);
    return call;
  }

EOF
((i++))
done

cat <<EOF
  // Loop while listening for completed responses.
  // Prints out the response from the server.
  void AsyncCompleteRpc() {
    void* got_tag;
    bool ok = false;

    // Block until the next result is available in the completion queue "cq".
    while (cq_.Next(&got_tag, &ok)) {
      // The tag in this example is the memory location of the call object
      ${service}AsyncClientCall* call = static_cast<${service}AsyncClientCall*>(got_tag);

      // Verify that the request was completed successfully. Note that "ok"
      // corresponds solely to the request for updates introduced by Finish().
      GPR_ASSERT(ok);

      if (!shutdown_
          && wait_for_ready_
          && !call->status.ok()
          && call->status.error_code() == grpc::UNAVAILABLE
          && call->retry_count < max_retry_count_) {

EOF

i=0
for rpc in ${rpcs[@]}; do
cat <<EOF
         if (call->rpcid == ${rpcid[i]}) ${rpcs[$i]}(call->${pref[i]}_request, call);
EOF
((i++))
done

cat <<EOF
      } else {
#if SEM_BUG
         call->sem.unlock();
#else
         call->sem.release();
#endif
      }
      if (shutdown_) break;
    }
  }

 private:
  // Out of the passed in Channel comes the stub, stored here, our view of the
  // server's exposed services.
  std::unique_ptr<${service}::Stub> stub_;

  // The producer-consumer queue we use to communicate asynchronously with the
  // gRPC runtime.
  CompletionQueue cq_;

  // thread to start AsyncCompleteRpc
  std::thread thread_;

  // indicate when to shutdown_ AsyncCompleteRpc
  bool shutdown_;

  // deadline for each RPC. Set it to -1 for a large deadline or large enough value.
  int deadline_ms_;

  // Flag indicating if we want to wait for server to be ready or fail fast.
  bool wait_for_ready_;

  // Number of retries for failed RPCs.
  // No need to implement the exponetial backoff for retries. The wait_for_ready flag
  // indirectly takes care of it.
  int max_retry_count_;

};

#if UNIT_TEST

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  std::string target_str = absl::GetFlag(FLAGS_target);
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials())


  // Start the client for the service ${service}
  // The client reconnects is the server restarts and retries the failed RPCs.
  ${service}Client *greeter = new ${service}Client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()),
      ${deadline_ms}, //deadline_ms
      ${wait_for_ready}, //wait_for_ready
      ${max_retry_count}); //max_retry_count

  int N = 100;
  int i;

  // Async client call contexts
  ${service}AsyncClientCall* call_contexts[N];

  for (i = 0; i < N; i++) {
    std::string user("world " + std::to_string(i));
       ${requests[1]} req;
       std::cout <<  "calling ${rpcs[1]} " << i << "\n";
       req.set_name(user); // FIXME for each RPC pack the request.
       call_contexts[i] = greeter->${rpcs[1]}(req);  // The RPC dispatch!
       std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }
  for (i = 0; i < N; i++) {
#if SEM_BUG
      call_contexts[i]->sem.lock();  //call complete
#else
      call_contexts[i]->sem.acquire(); // call complete
#endif
      if (call_contexts[i]->status.ok())
            std::cout << "${service} received: " << call_contexts[i]->${pref[1]}_reply.message() << std::endl;
      else
        std::cout << "RPC failed with " << call_contexts[i]->status.error_code() << std::endl;
  }
  std::cout << "Quitting\n";
  for (i = 0; i < N; i++) {

      // Once we're complete, deallocate the call object.
      delete call_contexts[i];
  }

  ${replies[0]} rep;
  ${requests[0]} req;
  std::string str("This is a test of sync call");
  req.set_name(str);
  std::cout << str << std::endl;
  int err;
  greeter->${rpcs[0]}(req, &rep, &err);
  if (err==0) {
      std::cout << "no error on async call\n";
      std::cout << "Sync reply: " << rep.message() << std::endl;
  }

  delete greeter;
  return 0;
}

#endif //UNIT_TEST
EOF

