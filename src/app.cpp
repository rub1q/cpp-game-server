#include "app.hpp"
#include "request_handler.hpp"
#include "config.hpp"
#include "endpoint.hpp"
#include "ticker.hpp"
#include "loot.hpp"
#include "serialization.hpp"

#include <vector>

#include <boost/asio/signal_set.hpp>

namespace app {

namespace net = boost::asio;
namespace sys = boost::system;

using namespace std::literals;

mux::Router App::get_router() const {
  mux::Router router;

  router.set_route(endpoint::GetIndex().route());
  router.set_route(endpoint::GetFile().route());
  router.set_route(endpoint::GetMapsList().route());
  router.set_route(endpoint::GameJoin().route());
  router.set_route(endpoint::GetPlayers().route());
  router.set_route(endpoint::GetMapInfo().route());
  router.set_route(endpoint::GetGameState().route());
  router.set_route(endpoint::PlayerAction().route());
  
  if (cfg_.env == config::AppEnv::test) {
    router.set_route(endpoint::Tick().route());
  }

  return router;
}

template <typename Fn>
void App::run_threads(const unsigned num, const Fn& fn) const {
  const auto max_threads = std::thread::hardware_concurrency(); 
  
  auto n = (num > max_threads) ? max_threads : num;

  std::vector<std::jthread> threads;
  threads.reserve(n-1);

  auto stop_token = stop_source_.get_token();

  while (n--) {
    threads.emplace_back(fn, stop_token);
  }

  fn(stop_token);
}

void App::run() {  
  const unsigned num_threads = std::thread::hardware_concurrency(); 
  net::io_context io { static_cast<int>(num_threads) };
  
  net::signal_set signals(io, SIGINT, SIGTERM);
  signals.async_wait([this, &io](const sys::error_code& ec, int signal_number) {
    if (!ec) {
      LOG_INFO << JSON_DATA({"signal"sv, signal_number}) 
        << "signal received"sv; 

      stop_source_.request_stop();
      io.stop();
    }
  });

  auto api_strand = net::make_strand(io);
  auto handler = std::make_shared<http_handler::RequestHandler>(api_strand, std::move(get_router()));

  const auto endpoint = http_server::tcp::endpoint(cfg_.server.addr, cfg_.server.port);

  http_handler::LoggingRequestHandler logging_handler {
    [handler](auto&& req, auto&& send) {
      return (*handler)(std::forward<decltype(req)>(req), std::forward<decltype(send)>(send));
    }
  };

  if (cfg_.env != config::AppEnv::test) {
    auto ticker = std::make_shared<gstime::Ticker>(api_strand, *cfg_.server.tick_period, [this](std::chrono::milliseconds delta) {
      const auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(delta);
      cfg_.game->refresh_state(dur.count());
    });

    ticker->start();
  }

  http_server::serve_http(io, std::move(endpoint), logging_handler);

  LOG_INFO << JSON_DATA(
    {"port"sv, cfg_.server.port},
    {"address"sv, cfg_.server.addr.to_string()}
  )
  << "server started"sv; 

  run_threads(num_threads, [&io](std::stop_token token) { 
    io.run(); 

    if (token.stop_requested()) {      
      LOG_INFO << "thread #0x"sv << std::hex 
        << std::this_thread::get_id() << " requested to stop"sv;
        
      return;
    }     
  });
}

} // namespace app