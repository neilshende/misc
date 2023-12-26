#include
 
<grpcpp/ext/proto_server_reflection_plugin.h>

#include
 
<thread>

#include <chrono>

// Replace with your actual service and stub classes
#include "your_service.grpc.pb.h"
using your::Service;
using your::Stub;

class RetryClient {
public:
    RetryClient(std::shared_ptr<grpc::Channel> channel) : stub_(Service::NewStub(channel)) {}

    void RetryCall(const grpc::ClientContext& context, const YourRequest& request, YourResponse* response) {
        int numRetries = 0;
        const int maxRetries = 3;
        const std::chrono::milliseconds initialBackoff = 500ms;

        while (numRetries < maxRetries) {
            grpc::Status status = stub_->YourRPC(&context, request, response);
            if (status.ok()) {
                return;  // Success!
            }

            numRetries++;
            std::cerr << "RPC failed with status: " << status.error_message() << std::endl;

            // Exponential backoff
            std::chrono::milliseconds backoff = initialBackoff * std::pow(2, numRetries - 1);
            std::this_thread::sleep_for(backoff);
        }

        // Exceeded max retries
        throw std::runtime_error("RPC failed after retries");
    }

private:
    std::unique_ptr<Stub> stub_;
};

int main() {
    std::string server_address("localhost:50051");  // Replace with your server address
    RetryClient client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

    YourRequest request;
    // ... fill in request details ...

    YourResponse response;
    grpc::ClientContext context;

    try {
        client.RetryCall(context, request, &response);
        // Handle successful response
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to get response after retries: " << e.what() << std::endl;
    }

    return 0;
}
