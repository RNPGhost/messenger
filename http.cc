#include "http.h"

#include <regex>

namespace {

const std::regex kRequestLinePattern{
    R"(^(GET|POST)\s+)"
    //  1
    R"(((([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)\s+)"
    // 234            5  6          7       8  9       10 11
    R"(HTTP/([0-9]+).([0-9]+))"};
    //     12       13

}  // namespace

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
