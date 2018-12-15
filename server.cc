#include "http.h"
#include "net.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <iterator>
#include <map>
#include <mutex>
#include <sstream>
#include <streambuf>
#include <string>
#include <thread>

struct Message {
  std::string sender_name;
  std::chrono::system_clock::time_point timestamp;
  std::string message;
};

struct MessageNode {
  Message message;
  std::shared_ptr<MessageNode> next;
};

class Server {
 public:
  void Run(const char* address, const char* service) {
    tcp::Acceptor acceptor = tcp::Bind(address, service);
    while(true) {
      tcp::Stream stream = acceptor.Accept();
      std::thread(&Server::HandleConnection, this, std::move(stream)).detach();
    }
  }

 private:
  void HandleConnection(tcp::Stream stream) {
    std::string input_string;
    while (true) {
      std::string read = stream.Read(1);

      if (read.empty()) {
        return;
      }

      input_string += read;

      if (input_string.length() >= 4 &&
          input_string.substr(input_string.length() - 4) == "\r\n\r\n") {
        break;
      }
    }

    std::istringstream input{input_string};
    Request request;
    input >> request;
    if (input.fail()) {
      std::cout << "input was not so great\n";
      return;
    }

    std::ostringstream output;
    output << GenerateResponse(request);

    std::string to_write = output.str();
    std::string_view to_write_view = to_write;

    while (to_write_view.length()) {
      int characters_written = stream.Write(to_write_view);
      to_write_view.remove_prefix(characters_written);
    }
  }

  Response GenerateResponse(const Request& request) {
    if (request.uri.path == "/") {
      std::ifstream index{"index.html"};
      std::string content{std::istreambuf_iterator<char>{index}, {}};
      return Response{Response::HttpStatus::kOk,
                      {{"Content-Type", "text/html; charset=utf-8"}},
                      content};
    }

    std::unique_lock lock{mutex_};
    return Response{Response::HttpStatus::kOk,
                    {{"Content-Type", "text/plain; charset=utf-8"}},
                    std::to_string(counters_[request.uri.path]++)};
  }

  std::mutex mutex_;
  std::map<std::string, int> counters_;
};

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "Usage: ./messenger <host> <port>\n";
    return 1;
  }

  Server server;
  server.Run(/*address=*/argv[1], /*service=*/argv[2]);
}

