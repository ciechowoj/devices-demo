#pragma once
#include <cstdint>
#include <cstddef>

namespace demo {

  struct message_t {
    uint64_t device_id = 0;
    uint64_t serial_id = 0;
    uint64_t timestamp = 0;
    int64_t measurement = 0;
  };

  constexpr size_t default_message_buffer_size = 256;

  std::string serialize_message(const message_t& message);
  size_t serialize_message(const message_t& message, char* buffer, size_t buffer_size);
  message_t deserialize_message(const char* buffer, size_t buffer_size);

}
