#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <unistd.h>

#include <grpcpp/grpcpp.h>

#include "chat.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;
using chat::ChatMessage;
using chat::ClientInfo;
using chat::Ack;
using chat::Chatter;

// Logic and data behind the server's behavior.
class ChatterServiceImpl final : public Chatter::Service {
public:

  Status RouteChat(ServerContext* context,
                   ServerReaderWriter<ChatMessage, ChatMessage>* stream) override  {
    
    std::cout << "New route chat received" << std::endl;

    this->streamMap.insert({context, stream});

    ChatMessage chatMsg;
    while (stream->Read(&chatMsg)) {
      std::cout << "message from: " << std::to_string(chatMsg.client_info().unique_id()) << ", " << chatMsg.msg() << std::endl;
      this->broadcast(chatMsg);
    }

    for(auto it = this->streamMap.begin(); it != this->streamMap.end(); it++) {
      if(it->first == context) {
        this->streamMap.erase(it);
      }

    }

    std::cout << "One route chat finished" << std::endl;

    return Status::OK;
  }

private:
 
  void broadcast(ChatMessage chatMsg) {
    for ( const auto &it : this->streamMap ) {
        it.second->Write(chatMsg);
    }
  }
  std::unordered_map<ServerContext*,ServerReaderWriter<ChatMessage, ChatMessage>*> streamMap;
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  ChatterServiceImpl service;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}