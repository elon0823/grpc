#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

#include "chat.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using chat::ChatMessage;
using chat::Ack;
using chat::Chatter;

class ChatterClient {
 public:
  ChatterClient(std::shared_ptr<Channel> channel)
      : stub_(Chatter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string SendChat(const std::string& user, const std::string& msg) {
    // Data we are sending to the server.
    ChatMessage request;
    request.set_name(user);
    request.set_msg(msg);

    // Container for the data we expect from the server.
    Ack reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SendChat(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return "Succeed";
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<Chatter::Stub> stub_;
};

int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  ChatterClient chatter(grpc::CreateChannel(
      "localhost:50051", grpc::InsecureChannelCredentials()));
  std::string user("world");
  std::string msg("life is good");
  std::string res = chatter.SendChat(user, msg);
  std::cout << "Chatter response: " << res << std::endl;

  return 0;
}