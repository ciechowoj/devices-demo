#include <iostream>
#include <message.hxx>
#include <cinttypes>
#include <cstdio>
#include <cstring>
#include <climits>
#include <stdexcept>

namespace demo {

  void serialize_message(
    const message_t& message,
    char* buffer,
    size_t buffer_size) {

    static const auto json_format =
      "{\n"
      R"(  "device_id": %)" PRIu64 ",\n"
      R"(  "serial_id": %)" PRIu64 ",\n"
      R"(  "timestamp": %)" PRIu64 ",\n"
      R"(  "measurement": %)" PRIi64 "\n"
      "%c";

    int result = snprintf(
      buffer,
      buffer_size,
      json_format,
      message.device_id,
      message.serial_id,
      message.timestamp,
      message.measurement,
      '}');
  }

  [[noreturn]] void parse_error() {
    throw std::runtime_error("The message is corrupted!");
  }

  void skip_whitespaces(const char*& itr, const char* end) {
    while (itr < end && std::isspace(*itr)) {
      ++itr;
    }
  }

  void parse_char(const char*& itr, const char* end, char c) {
    if (itr < end && *itr == c) {
      ++itr;
    }
    else {
      parse_error();
    }
  }

  void parse_label(const char*& itr, const char* end, char* label, size_t label_size) {
    parse_char(itr, end, '"');

    auto label_end = label + label_size;

    while (itr < end && label + 1 < label_end && std::isprint(*itr) && *itr != '"') {
      *label = *itr;
      ++itr;
      ++label;
    }

    *label = 0;

    if (itr < end && *itr == '"') {
      ++itr;
    }
    else {
      parse_error();
    }

    skip_whitespaces(itr, end);
    parse_char(itr, end, ':');
  }

  void parse_unumber(const char*& itr, const char* end, uint64_t& result) {
    if (itr >= end || !std::isdigit(*itr)) {
      parse_error();
    }

    char* jtr = nullptr;
    auto local_result = strtoull(itr, &jtr, 10);

    if (local_result == ULLONG_MAX && errno == ERANGE) {
      throw std::out_of_range("integer out of range");
    }

    itr = jtr;
    result = local_result;
  }

  void parse_inumber(const char*& itr, const char* end, int64_t& result) {
    if (itr >= end || !std::isdigit(*itr) && *itr != '-') {
      parse_error();
    }

    char* jtr = nullptr;
    auto local_result = strtoll(itr, &jtr, 10);

    if ((local_result == LLONG_MAX || local_result == LLONG_MIN) && errno == ERANGE) {
      throw std::out_of_range("integer out of range");
    }

    itr = jtr;
    result = local_result;
  }

  message_t deserialize_message(const char* buffer, size_t buffer_size) {
    message_t message;

    const char* itr = buffer;
    const char* end = itr + buffer_size;

    parse_char(itr, end, '{');
    skip_whitespaces(itr, end);

    constexpr size_t label_size = 1024;
    char label[label_size];

    constexpr int device_id_flag = 1;
    constexpr int serial_id_flag = 2;
    constexpr int timestamp_flag = 4;
    constexpr int measurement_flag = 8;

    constexpr int expected_mask =
      device_id_flag | serial_id_flag | timestamp_flag | measurement_flag;

    int actual_mask = 0;

    while (itr < end) {
      parse_label(itr, end, label, label_size);
      skip_whitespaces(itr, end);

      if (strcmp(label, "device_id") == 0) {
        parse_unumber(itr, end, message.device_id);
        actual_mask |= device_id_flag;
      }
      else if (strcmp(label, "serial_id") == 0) {
        parse_unumber(itr, end, message.serial_id);
        actual_mask |= serial_id_flag;
      }
      else if (strcmp(label, "timestamp") == 0) {
        parse_unumber(itr, end, message.timestamp);
        actual_mask |= timestamp_flag;
      }
      else if (strcmp(label, "measurement") == 0) {
        parse_inumber(itr, end, message.measurement);
        actual_mask |= measurement_flag;
      }
      else {
        parse_error();
      }

      skip_whitespaces(itr, end);

      if (itr < end && *itr == '}') {
        ++itr;
        break;
      }
      else {
        parse_char(itr, end, ',');
        skip_whitespaces(itr, end);
      }
    }

    if (itr != end || expected_mask != actual_mask) {
      parse_error();
    }

    return message;
  }
}
