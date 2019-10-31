#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <random>

#include <grpcpp/grpcpp.h>
#include <grpcpp/client_context.h>

#include "chat.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;
using grpc::ClientReader;
using grpc::Status;
using chat::ChatMessage;
using chat::ClientInfo;
using chat::Ack;
using chat::Chatter;

class ChatterClient {
 public:
  ChatterClient(std::shared_ptr<Channel> channel)
      : stub_(Chatter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  void Chatting() {

    ClientContext context;
    
    ClientInfo client_info;
    std::random_device rn;
    std::mt19937 rnd( rn() );
    
    std::uniform_int_distribution<uint32_t> range(0, 0xffffffff);
    client_info.set_unique_id(range(rnd));

    std::shared_ptr<ClientReaderWriter<ChatMessage, ChatMessage> > stream(
        stub_->RouteChat(&context));

    std::thread writer([&]() {
      std::string msg;
      std::string user("world");

      while(true) {
        
        std::cin >> msg;
        
        if(msg.compare("exit") == 0) {
          stream->WritesDone();
          
          break;
        }
        else {
          ChatMessage chatMsg;
          chatMsg.mutable_client_info()->CopyFrom(client_info);
          chatMsg.set_name(user);
          chatMsg.set_msg(msg);

          stream->Write(chatMsg);
        }
      }
    });

    ChatMessage received_msg;
    while (stream->Read(&received_msg)) {
      std::cout << "message from: " << std::to_string(received_msg.client_info().unique_id()) << ", " << received_msg.msg() << std::endl;
    }

    writer.join();    

    Status status = stream->Finish();
    if (!status.ok()) {
      std::cout << "RouteChat rpc failed." << std::endl;
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
  
  chatter.Chatting();

  return 0;
}