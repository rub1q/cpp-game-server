#pragma once 

#include "config.hpp"
#include "logger.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/dispatch.hpp>

namespace http_server {

namespace net = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;

using tcp = net::ip::tcp;

using namespace std::literals;

class SessionBase  {
public:
  SessionBase(const SessionBase&) = delete;
  SessionBase& operator=(const SessionBase&) = delete;

  void run() {
    net::dispatch(
      stream_.get_executor(), 
      beast::bind_front_handler(&SessionBase::read, get_shared_from_this())
    );
  }

protected:
  using HttpRequest = http::request<http::string_body>;

  ~SessionBase() = default; 

  explicit SessionBase(tcp::socket&& socket)
    : stream_(std::move(socket)) {
  }

  template <typename Body, typename Fields>
  void write(http::response<Body, Fields>&& response) {
    auto safe_response = std::make_shared<http::response<Body, Fields>>(std::move(response));

    stream_.expires_after(config::get().server.write_timeout);

    http::async_write(stream_, *safe_response, 
      [safe_response, self = get_shared_from_this()](beast::error_code ec, std::size_t bytes_written) {
        self->on_write(safe_response->need_eof(), ec, bytes_written);
      });
  }

  tcp::socket::endpoint_type remote_endpoint() const {
    return stream_.socket().remote_endpoint();
  }

private:
  void read() {
    request_ = {};
    stream_.expires_after(config::get().server.read_timeout);

    http::async_read(stream_, buf_, request_,
      beast::bind_front_handler(&SessionBase::on_read, get_shared_from_this()));
  }

  void on_read(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read) {
    if (ec) {
      if (ec == http::error::end_of_stream) {
        return close();
      }
      
      LOG_SYSTEM_ERROR(ec.value(), ec.message())
      return;
    }

    handle_request(std::move(request_));
  }

  void close() {
    beast::error_code ec; 
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
  }

  void on_write(bool close_socket, beast::error_code ec, [[maybe_unused]] std::size_t bytes_written) {
    if (close_socket) {
      return close();
    }

    if (ec) {
      LOG_SYSTEM_ERROR(ec.value(), ec.message())
      return;
    }

    read();
  }

  virtual void handle_request(HttpRequest&& request) = 0;
  virtual std::shared_ptr<SessionBase> get_shared_from_this() = 0;

private:
  beast::tcp_stream stream_;
  beast::flat_buffer buf_;

  HttpRequest request_;
};

template <typename RequestHandler>
class Session : public SessionBase, public std::enable_shared_from_this<Session<RequestHandler>> {
public:
  template <typename Handler>
  Session(tcp::socket&& socket, Handler&& handler) 
    : SessionBase(std::move(socket))
    , request_handler_(std::forward<Handler>(handler)) {
  }

private:
  std::shared_ptr<SessionBase> get_shared_from_this() override {
    return this->shared_from_this();
  }

  void handle_request(HttpRequest&& request) override {
    request_handler_(remote_endpoint(), std::move(request), [self = this->shared_from_this()](auto&& response) {
      self->write(std::move(response));
    });
  }

private:
  RequestHandler request_handler_;
};

} // namespace http_server