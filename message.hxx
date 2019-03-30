#include <cstdint>
#include <cstddef>

namespace demo {

  struct message_t {
    uint64_t device_id = 0;
    uint64_t serial_id = 0;
    uint64_t timestamp = 0;
    int64_t measurement = 0;
  };

  void serialize_message(const message_t& message, char* buffer, size_t buffer_size);
  message_t deserialize_message(const char* buffer, size_t buffer_size);

}
