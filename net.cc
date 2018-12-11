#include "net.h"

#include <unistd.h>
#include <netdb.h>
#include <system_error>

using namespace std::literals;

namespace tcp {

static std::system_error errno_code(std::string message) {
  return std::system_error{errno, std::system_category(), message};
}

static void throw_gai_error(int code) {
  if (code == EAI_SYSTEM) {
    throw errno_code("getaddrinfo");
  } else {
    throw std::runtime_error("getaddrinfo: "s + gai_strerror(code));
  }
}

Socket::Socket(int handle) noexcept : handle_{handle} {}

Socket::Socket(Socket&& other) noexcept : handle_{other.handle_} {
  other.handle_ = -1;
}

Socket::~Socket() noexcept {
  if (handle_ != -1) ::close(handle_);
}

Socket& Socket::operator=(Socket&& other) noexcept {
  if (handle_ != -1) {
    ::close(handle_);
  }

  handle_ = other.handle_;
  other.handle_ = -1;

  return *this;
}

Stream::Stream(Socket socket) noexcept : socket_{std::move(socket)} {}

std::string Stream::Read(int max_length) {
  std::string buffer(max_length, ' ');
  int read_size = ::read(socket_.handle(), buffer.data(), max_length);
  if (read_size == -1) throw errno_code("Read Failed");
  buffer.resize(read_size);
  return buffer;
}

int Stream::Write(std::string_view message) {
  int write_size = ::write(socket_.handle(), message.data(), message.length());
  if (write_size == -1) throw errno_code("Write Failed");
  return write_size;
}

void Stream::Close() {
  int not_size = ::close(socket_.handle());
  if (not_size == -1) throw errno_code("Close Failed");
}

Acceptor::Acceptor(Socket socket) noexcept : socket_{std::move(socket)} {}

Stream Acceptor::Accept() {
  int accept_socket = ::accept(socket_.handle(), nullptr, nullptr);
  if (accept_socket == -1) throw errno_code("Accept Failed");
  return Stream{Socket{accept_socket}};
}

void Acceptor::Close() {
  int not_size = ::close(socket_.handle());
  if (not_size == -1) throw errno_code("Close Failed");
}

Acceptor Bind(std::string address, std::string service) {
  ::addrinfo* addr_info = nullptr;
  int status = ::getaddrinfo(address.c_str(), service.c_str(), nullptr,
                             &addr_info);
  if (status != 0) throw_gai_error(status);

  Socket socket{::socket(addr_info->ai_family, SOCK_STREAM, 0)};
  if (socket.handle() == -1) throw errno_code("Socket Creation Failed");

  int result = ::bind(socket.handle(), addr_info->ai_addr,
      addr_info->ai_addrlen);
  if (result == -1) throw errno_code("Bind Failed");

  result = ::listen(socket.handle(), Acceptor::kMaxPendingConnections);
  if (result == -1) throw errno_code("Listen Failed");

  return Acceptor{std::move(socket)};
}

Stream Connect(std::string address, std::string service) {
  ::addrinfo* addr_info = nullptr;
  int status = ::getaddrinfo(address.c_str(), service.c_str(), nullptr,
                             &addr_info);
  if (status != 0) throw_gai_error(status);

  Socket socket{::socket(addr_info->ai_family, SOCK_STREAM, 0)};
  if (socket.handle() == -1) throw errno_code("Socket Creation Failed");

  int result = ::connect(socket.handle(), addr_info->ai_addr,
      addr_info->ai_addrlen);
  if (result == -1) throw errno_code("Connect Failed");

  return Stream{std::move(socket)};
}

}  // namespace tcp
