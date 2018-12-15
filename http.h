#pragma once

#include <iostream>
#include <map>
#include <string>

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

std::ostream& operator<<(std::ostream& output, const Response& response);
std::istream& operator>>(std::istream& input, Request& request);
