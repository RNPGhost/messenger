#pragma once

#include <string>
#include <string_view>

namespace tcp {

class Socket {
 public:
  explicit Socket(int handle) noexcept;
  ~Socket() noexcept;

  // Not copyable
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  // Movable
  Socket(Socket&&) noexcept;
  Socket& operator=(Socket&&) noexcept;

  int handle() const noexcept { return handle_; }

 private:
  int handle_;
};

class Stream {
 public:
  explicit Stream(Socket socket) noexcept;

  [[nodiscard]] std::string Read(int max_length);
  [[nodiscard]] int Write(std::string_view content);
  void Close();

 private:
  Socket socket_;
};

class Acceptor {
 public:
  static constexpr int kMaxPendingConnections = 7;
  explicit Acceptor(Socket socket) noexcept;
  [[nodiscard]] Stream Accept();
  void Close();
 private:
  Socket socket_;
};

Stream Connect(std::string address, std::string service);

Acceptor Bind(std::string address, std::string service);

}  // namespace tcp

