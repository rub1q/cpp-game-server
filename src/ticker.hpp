#pragma once 

#include <chrono>
#include <boost/asio/strand.hpp>
#include <boost/asio/steady_timer.hpp>

namespace gstime {

namespace net = boost::asio;
namespace sys = boost::system;

using namespace std::chrono;

class Ticker : public std::enable_shared_from_this<Ticker> {
public:
  using Strand = net::strand<net::io_context::executor_type>; 
  using Handler = std::function<void(milliseconds delta)>;

  explicit Ticker(Strand strand, milliseconds period, Handler&& handler)
    : strand_(strand)
    , period_(period)
    , handler_(std::move(handler)) {
  }

  void start();

private:
  void shedule_tick();
  void on_tick(sys::error_code ec);

private:
  Strand strand_;
  Handler handler_;
  net::steady_timer timer_ { strand_ };

  milliseconds period_;
  steady_clock::time_point last_tick_;
};

} // namespace gstime