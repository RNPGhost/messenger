#include <iostream>
#include <map>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

#include "net.h"

const std::regex kRequestLinePattern{
    R"(^(GET|POST)\s+)"
    //  1
    R"(((([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)\s+)"
    // 234            5  6          7       8  9       10 11
    R"(HTTP/([0-9]+).([0-9]+))"};
    //     12       13


std::mutex mutex;
std::map<std::string, int> counters;

struct Uri {
  std::string path;
  std::string query;
};

struct HttpVersion {
  int major;
  int minor;
};

struct Request {
  enum class HttpType {
    kGet,
    kPost,
  };

  HttpType type;
  HttpVersion http_version;
  Uri uri;
  std::map<std::string, std::string> headers;
  std::string body;
};

struct Response {
  enum class HttpStatus {
    kOk = 200,
    kNotFound = 404,
    kLolDicks = 500,
  };

  HttpStatus status;
  std::map<std::string, std::string> headers;
  std::string body;
};

std::ostream& operator<<(std::ostream& output, const Response& response) {
  output << "HTTP/1.1 " << static_cast<int>(response.status)
         << " gigglepigs\r\n";

  for (auto&& [key, value] : response.headers) {
    if (key != "Content-Length") {
      output << key << ": " << value << "\r\n";
    }
  }

  output << "Content-Length: " << response.body.length() << "\r\n\r\n"
         << response.body;

  return output;
}

std::istream& operator>>(std::istream& input, Request& request) {
  std::string request_line;
  std::getline(input, request_line, '\r');

  std::cout << request_line << "\n";

  std::smatch match;
  if (!std::regex_match(request_line, match, kRequestLinePattern)) {
    input.setstate(std::ios::failbit);
    return input;
  }

  std::string request_type = match[1];
  if (request_type == "GET") {
    request.type = Request::HttpType::kGet;
  } else if (request_type == "POST") {
    request.type = Request::HttpType::kPost;
  } else {
    input.setstate(std::ios::failbit);
    return input;
  }

  if (!match[3].str().empty() && match[4] != "http") {
    input.setstate(std::ios::failbit);
    return input;
  }

  request.uri.path = match[7];
  request.uri.query = match[9];

  request.http_version.major = std::stoi(match[12]);
  request.http_version.minor = std::stoi(match[13]);

  return input;
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

  std::istringstream input{input_string};
  Request request;
  input >> request;
  if (input.fail()) {
    std::cout << "input was not so great\n";
    return;
  }

  std::unique_lock lock{mutex};
  Response response{Response::HttpStatus::kOk,
                    {{"Content-Type", "text/plain; charset=utf-8"}},
                    std::to_string(counters[request.uri.path]++)};


  std::ostringstream output;

  output << response;

  std::string to_write = output.str();
  std::string_view to_write_view = to_write;

  while (to_write_view.length()) {
    int characters_written = stream.Write(to_write_view);
    to_write_view.remove_prefix(characters_written);
  }
}

int main(int argc, char* argv[]) {
 if (argc != 3) {
    std::cout << "Usage: ./messenger <host> <port>\n";
    return 1;
  }

  tcp::Acceptor acceptor = tcp::Bind(/*address=*/argv[1], /*service=*/argv[2]);
  while(true) {
    tcp::Stream stream = acceptor.Accept();
    std::thread(HandleConnection, std::move(stream)).detach();
  }
}

