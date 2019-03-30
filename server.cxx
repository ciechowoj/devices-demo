#include <cstdlib>
#include <iostream>
#include <asio.hpp>
#include <message.hxx>

namespace demo {

  using asio::ip::udp;

  class udp_server_t {
  public:
    udp_server_t(asio::io_context& io_context, short port)
      : socket_(io_context, udp::endpoint(udp::v4(), port)) {
      do_receive();
    }

    void do_receive() {

      auto closure = [this](std::error_code ec, std::size_t bytes_recvd) {
        if (!ec && bytes_recvd > 0) {
          data_[bytes_recvd] = 0;

          std::cout << data_ << std::endl;
        }

        do_receive();
      };

      socket_.async_receive_from(
        asio::buffer(data_, max_length),
        sender_endpoint_,
        closure);
    }

    virtual void on_receive(const std::string_view& message) = 0;

  private:
    udp::socket socket_;
    udp::endpoint sender_endpoint_;
    enum { max_length = 1024 };
    char data_[max_length];
  };

  class message_server_t : private udp_server_t {
  public:
    message_server_t(asio::io_context& io_context)
      : udp_server_t(io_context, _port_number) {
      do_receive();
    }

    virtual void on_receive(const message_t& message) = 0;

  private:
    static constexpr short _port_number = 1911;

    void on_receive(const std::string_view& message) override {

    }

  };

  class message_counter_t : public message_server_t {
  public:
    message_counter_t(asio::io_context& io_context)
      : message_server_t(io_context) {
    }


    void on_receive(const message_t& message) override {
      ++_message_count;
    }

  private:
    std::atomic<uint64_t> _message_count;
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
