#include <cstdlib>
#include <cstring>
#include <iostream>
#include <asio.hpp>
#include <cmath>
#include <time.h>

using asio::ip::udp;

enum { max_length = 1024 };

void device_sleep(double seconds) {
  double integral, fractional;
  fractional = modf(seconds, &integral);

  struct timespec rem;

  struct timespec req =  {
    time_t(integral),
    long(fractional * 1'000'000'000)
  };

  int result = -1;
  errno = EINTR;

  while (result == -1 && errno == EINTR) {
    result = nanosleep(&req, &rem);
    req = rem;
  }
}

int main(int argc, char* argv[]) {
  try
  {
    asio::io_context io_context;

    auto socket = udp::socket(io_context, udp::endpoint(udp::v4(), 0));

    const char* host = argv[1];
    const char* port = argv[2];

    auto resolver = udp::resolver(io_context);
    auto endpoints = resolver.resolve(udp::v4(), host, port);

    while (true) {
      auto request = "Hello, world!";
      size_t request_size = std::strlen(request);
      socket.send_to(asio::buffer(request, request_size), *endpoints.begin());
      std::cout << request << std::endl;
      device_sleep(3);
    }
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
