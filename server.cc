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
  using Handler = Response (Server::*)(const Request&);

  Response ErrorPage(const Request&, Response::HttpStatus status) {
    return Response{status,
                    {{"Content-Type", "text/plain; charset=utf-8"}},
                    ":("};
  }

  Response HandleMessage(const Request& request) {
    return Response{Response::HttpStatus::kOk,
                    {{"Content-Type", "text/html; charset=utf-8"}},
                    request.body};
  }

  Response IndexPage(const Request&) {
    std::ifstream index{"index.html"};
    std::string content{std::istreambuf_iterator<char>{index}, {}};
    return Response{Response::HttpStatus::kOk,
                    {{"Content-Type", "text/html; charset=utf-8"}},
                    content};
  }

  Response Style(const Request&) {
    std::ifstream index{"style.css"};
    std::string content{std::istreambuf_iterator<char>{index}, {}};
    return Response{Response::HttpStatus::kOk,
                    {{"Content-Type", "text/css; charset=utf-8"}},
                    content};
  }

  Response Script(const Request&) {
    std::ifstream index{"script.js"};
    std::string content{std::istreambuf_iterator<char>{index}, {}};
    return Response{Response::HttpStatus::kOk,
                    {{"Content-Type",
                      "application/x-javascript; charset=utf-8"}},
                    content};
  }


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

    std::istringstream input{input_string.substr(0, input_string.length() - 4)};
    Request request;
    input >> request;
    if (input.fail()) {
      std::cerr << "input was not so great\n";
      return;
    }

    auto i = request.headers.find("content-length");
    if (i != request.headers.end()) {
      int message_length = -1;
      try {
        message_length = std::stoi(i->second);
      } catch (...) {}
      if (message_length > 0 && message_length < 9001) {
        request.body = stream.Read(message_length);
        std::cout << request.body << "\n";
      } else if (message_length != 0) {
        std::cerr << "content length was not so great\n";
        return;
      }
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
    auto i = path_map_.find(request.uri.path);
    if (i == path_map_.end()) {
      return ErrorPage(request, Response::HttpStatus::kNotFound);
    } else {
      return (this->*i->second)(request);
    }
  }

  std::mutex mutex_;
  std::map<std::string, int> counters_;
  std::map<std::string, Handler> path_map_ = {
      {"/", &Server::IndexPage},
      {"/message", &Server::HandleMessage},
      {"/script.js", &Server::Script},
      {"/style.css", &Server::Style}
  };
};

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cout << "Usage: ./messenger <host> <port>\n";
    return 1;
  }

  Server server;
  server.Run(/*address=*/argv[1], /*service=*/argv[2]);
}

