#pragma once 

#include "session.hpp"
#include "logger.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>

namespace http_server {

namespace net = boost::asio;
namespace beast = boost::beast;
namespace sys = boost::system;

using tcp = net::ip::tcp;

template <typename RequestHandler>
class Listener : public std::enable_shared_from_this<Listener<RequestHandler>> {
public: 

  template <typename Handler>
  Listener(net::io_context& io, const tcp::endpoint& endpoint, Handler&& handler)
    : io_(io)
    , acceptor_(net::make_strand(io))
    , request_handler_(std::forward<Handler>(handler)) {
    
    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(net::socket_base::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen(net::socket_base::max_listen_connections);
  }

  void run() {
    do_accept();
  }

private:
  void do_accept() {
    acceptor_.async_accept(
      net::make_strand(io_),
      beast::bind_front_handler(&Listener::on_accept, this->shared_from_this())
    );
  }

  void on_accept(sys::error_code ec, tcp::socket socket) {
    using namespace std::literals;
    
    if (ec) {
      LOG_SYSTEM_ERROR(ec.value(), ec.message())
      return;
    }

    run_session(std::move(socket));
    do_accept();
  }

  void run_session(tcp::socket&& socket) {
    auto session = std::make_shared<Session<RequestHandler>>(std::move(socket), request_handler_);
    session->run();
  }

private:
  net::io_context& io_;
  tcp::acceptor acceptor_;

  RequestHandler request_handler_;
};

} // namespace http_server

namespace http_server {

template <typename RequestHandler>
void serve_http(net::io_context& io, const tcp::endpoint& endpoint, RequestHandler&& handler) {
  using MyListener = Listener<std::decay_t<RequestHandler>>;

  auto listener = std::make_shared<MyListener>(io, endpoint, std::forward<RequestHandler>(handler));
  listener->run();
}

} // namespace http_server