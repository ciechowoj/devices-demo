#include <cstdlib>
#include <iostream>
#include <asio.hpp>
#include <message.hxx>
#include <unordered_map>

namespace demo {

  using asio::ip::udp;

  class udp_server_t {
  public:
    udp_server_t(asio::io_context& io_context, short port)
      : _socket(io_context, udp::endpoint(udp::v4(), port)) {
      static_assert(default_message_buffer_size < _max_size);
      io_context.post([this] { do_receive(); });
    }

    virtual void on_receive(const std::string_view& message) = 0;

  private:
    void do_receive() {
      auto closure = [this](std::error_code ec, std::size_t bytes_recvd) {
        if (!ec && bytes_recvd > 0) {
          on_receive(std::string_view(_data, bytes_recvd));
        }

        do_receive();
      };

      _socket.async_receive_from(
        asio::buffer(_data, _max_size),
        _sender_endpoint,
        closure);
    }

    udp::socket _socket;
    udp::endpoint _sender_endpoint;
    static constexpr size_t _max_size = 1024;
    char _data[_max_size];
  };

  class message_server_t : private udp_server_t {
  public:
    message_server_t(asio::io_context& io_context)
      : udp_server_t(io_context, _port_number) { }

    virtual void on_receive(const message_t& message, uint64_t prev_serial_id) = 0;

  private:
    struct _device_state_t {
      uint64_t serial_id = 0;
      uint64_t timestamp = 0;
    };

    void on_receive(const std::string_view& raw_message) override {
      auto message = deserialize_message(raw_message.data(), raw_message.size());

      bool repeated = false;

      auto prev_serial_id = message.serial_id;

      if (auto lock = std::lock_guard<std::mutex>(_latest_serial_id_mutex); true) {
        auto itr = _latest_serial_ids.find(message.device_id);

        if (itr == _latest_serial_ids.end()) {
          _device_state_t state;
          state.serial_id = message.serial_id;
          state.timestamp = message.timestamp;
          _latest_serial_ids.emplace(message.device_id, state);
        }
        else {
          if (itr->second.serial_id < message.serial_id && itr->second.timestamp <= message.timestamp) {
            prev_serial_id = itr->second.serial_id;
            itr->second.serial_id = message.serial_id;
            itr->second.timestamp = message.timestamp;
          }
          else if (itr->second.serial_id >= message.serial_id && itr->second.timestamp < message.timestamp) {
            // if timestamps are consecutive and serial_ids reversed
            // it means that the device was restarted and the latest serial_id
            // stored in the hash map is invalid
            itr->second.serial_id = message.serial_id;
            itr->second.timestamp = message.timestamp;
          }
          else {
            repeated = true;
          }
        }
      }

      if (!repeated) {
        on_receive(message, prev_serial_id);
      }
    }

    std::unordered_map<uint64_t, _device_state_t> _latest_serial_ids;
    std::mutex _latest_serial_id_mutex;

    static constexpr short _port_number = 1911;
  };

  class message_counter_t : public message_server_t {
  public:
    message_counter_t(asio::io_context& io_context)
      : message_server_t(io_context)
      , _timer(io_context) {
      _received_message_count = 0;
      _lost_message_count = 0;
      _timer.expires_after(std::chrono::seconds(1));
      _timer.async_wait([this] (auto ec) { on_timer(ec); });
    }

    void on_receive(const message_t& message, uint64_t prev_serial_id) override {
      ++_received_message_count;
      _lost_message_count += std::max(uint64_t(1), message.serial_id - prev_serial_id) - 1;
    }

    void on_timer(const std::error_code& ec) {
      if (!ec) {
        std::cout
          << R"({ "received": )"
          << _received_message_count
          << R"(, "lost": )"
          << _lost_message_count
          << " }"
          << std::endl;
      }

      _timer.expires_at(_timer.expiry() + asio::chrono::seconds(1));
      _timer.async_wait([this] (auto ec) { on_timer(ec); });
    }

  private:
    asio::steady_timer _timer;
    std::atomic<uint64_t> _received_message_count;
    std::atomic<uint64_t> _lost_message_count;
  };

}

int main(int argc, char* argv[]) {
  try {
    asio::io_context io_context;

    demo::message_counter_t server(io_context);

    io_context.run();
  }
  catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
