#include <iostream>
#include <unittest.hxx>
#include <message.hxx>
#include <cstring>
using namespace demo;
using namespace haste;

unittest("Test serialize/deserialize roundtrip.") {
  char buffer[1024];

  auto message = message_t { 1, 2, 3, 4 };
  serialize_message(message, buffer, sizeof(buffer));
  auto actual = deserialize_message(buffer, std::strlen(buffer));

  assert_eq(1, actual.device_id);
  assert_eq(2, actual.serial_id);
  assert_eq(3, actual.timestamp);
  assert_eq(4, actual.measurement);
}

unittest("Throw if the buffer is not null terminated.") {
  auto message = R"({"device_id":1,"serial_id":2,"timestamp":3,"measurement":4})";

  char buffer[1024];

  std::memset(buffer, 1, sizeof(buffer));
  std::memcpy(buffer, message, std::strlen(message));

  assert_throws([&] { deserialize_message(buffer, sizeof(buffer)); });
}

unittest("Throw if the buffer is corrupted.") {
  auto message0 = R"({"device_id":1,"serial_id":2,"timestamp":3,"measu)";
  assert_throws([&] { deserialize_message(message0, std::strlen(message0)); });

  auto message1 = R"({"device_id":1,"serial_id":2,"timestxmp":3,"measurement":4})";
  assert_throws([&] { deserialize_message(message1, std::strlen(message1)); });

  auto message2 = R"("device_id":1,"serial_id":2,"timestxmp":3,"measurement":4})";
  assert_throws([&] { deserialize_message(message2, std::strlen(message2)); });
}

unittest("Throw if the buffer is corrupted (integer overflow).") {
  auto message0 =
    R"({"device_id":1,"serial_id":2,"timestamp":18446744073709551615184467440737,"measurement":4})";
  assert_throws([&] { deserialize_message(message0, std::strlen(message0)); });

  auto message1 =
    R"({"device_id":1,"serial_id":2,"timestamp":3,"measurement":-18446744073709551615})";
  assert_throws([&] { deserialize_message(message1, std::strlen(message1)); });
}

unittest("Throw if the buffer is corrupted (negative serial_id).") {
  auto message0 =
    R"({"device_id":1,"serial_id":-2,"timestamp":1844674,"measurement":4})";
  assert_throws([&] { deserialize_message(message0, std::strlen(message0)); });
}

unittest("Parse negative measurement.") {
  auto message0 =
    R"({"device_id":1,"serial_id":2,"timestamp":1844674,"measurement":-9223372036854775806})";
  auto message = deserialize_message(message0, std::strlen(message0));

  assert_eq(-9223372036854775806, message.measurement);
}

unittest("Parse reordered json members measurement.") {
  auto message0 =
    R"({"device_id":1,"timestamp":1844674,"serial_id":2,"measurement":-42})";
  auto actual = deserialize_message(message0, std::strlen(message0));

  assert_eq(1, actual.device_id);
  assert_eq(2, actual.serial_id);
  assert_eq(1844674, actual.timestamp);
  assert_eq(-42, actual.measurement);
}

unittest("Throw if any of the fields is missing.") {
  auto message0 =
    R"({"timestamp":1844674,"serial_id":2,"measurement":-42})";
  assert_throws([&] { deserialize_message(message0, std::strlen(message0)); });

  auto message1 =
    R"({"device_id":1,"serial_id":2,"measurement":-42})";
  assert_throws([&] { deserialize_message(message1, std::strlen(message1)); });

  auto message2 =
    R"({"device_id":1,"timestamp":1844674,"measurement":-42})";
  assert_throws([&] { deserialize_message(message2, std::strlen(message2)); });

  auto message3 =
    R"({"device_id":1,"timestamp":1844674,"serial_id":2})";
  assert_throws([&] { deserialize_message(message3, std::strlen(message3)); });
}

unittest("default_message_buffer_size is correct") {
  // Assuming one doesn't go crazy with the whitespaces in the json,
  // the default_message_buffer_size should fit all possible messages.

  auto message3 =
    R"({"device_id":18446744073709551614,"timestamp":18446744073709551614,"serial_id":18446744073709551614,"measurement":-9223372036854775806})";

  assert_true(default_message_buffer_size > std::strlen(message3));

  deserialize_message(message3, std::strlen(message3));
}


