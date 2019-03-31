#include <cstdlib>
#include <cstring>
#include <iostream>
#include <asio.hpp>
#include <cmath>
#include <time.h>
#include <message.hxx>

using asio::ip::udp;

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

uint64_t monotonic_ns() {
  struct timespec ts;
  const clockid_t clk_id = CLOCK_MONOTONIC;

  if (clock_gettime(clk_id, &ts) != 0) {
    throw std::runtime_error("failed to get monotonic time");
  }

  uint64_t result = ts.tv_sec;
  if (result > ULLONG_MAX / 1'000'000'000) {
    throw std::runtime_error("timestamp too large to convert to uint64_t");
  }

  result *= 1'000'000'000;

  if (result > ULLONG_MAX - ts.tv_nsec) {
    throw std::runtime_error("timestamp too large to convert to uint64_t");
  }

  result += ts.tv_nsec;

  return result;
}

int main(int argc, char* argv[]) {
  try {
    asio::io_context io_context;

    auto socket = udp::socket(io_context, udp::endpoint(udp::v4(), 0));

    const char* host = argv[1];
    const char* port = DEMO_PORT;

    char* end = nullptr;
    const uint64_t device_id = std::strtoull(argv[2], &end, 10);
    const double period = std::strtod(argv[3], &end);
    const bool print = argc > 4 && std::strcmp(argv[4], "true") == 0;

    auto resolver = udp::resolver(io_context);
    auto endpoints = resolver.resolve(udp::v4(), host, port);

    uint64_t serial_id = 0;

    while (true) {
      char request[demo::default_message_buffer_size];
      demo::message_t message;

      message.device_id = device_id;
      message.serial_id = serial_id++;
      message.timestamp = monotonic_ns();
      message.measurement = std::rand();

      auto request_size = demo::serialize_message(
        message,
        request,
        demo::default_message_buffer_size);

      socket.send_to(asio::buffer(request, request_size), *endpoints.begin());

      if (print) {
        std::cout << request << std::endl;
      }

      device_sleep(period);
    }
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
