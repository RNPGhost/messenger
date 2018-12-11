#include <iostream>

#include "net.h"

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "Usage: ./messenger <host> <port>\n";
    return 1;
  }

  tcp::Stream stream = tcp::Connect(/*address=*/argv[1], /*service=*/argv[2]);
  int write_size = stream.Write("fISHY cRAcKERS\n");
  std::cout << write_size << "\n";
  // i'm happy
}

/*
socket() returns int

client sockets:
  connect(socket, address)
  read() returns int
  write() returns int
server sockets:
  bind(socket, address) -> port
  listen(socket, number_of_pending) -> start listening
  accept(socket) returns a socket -> accept a single connection
*/
