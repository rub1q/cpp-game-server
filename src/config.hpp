#pragma once 

#include "game.hpp"
#include "cli.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <chrono>
#include <memory>

namespace config {

namespace net = boost::asio;
namespace fs = std::filesystem;

using clock = std::chrono::steady_clock;
using namespace std::literals;

struct ServerConfig {
  net::ip::port_type port { 8080u };
  net::ip::address addr { net::ip::make_address("0.0.0.0"sv) };

  clock::duration read_timeout  { 15s };
  clock::duration write_timeout { 15s };

  fs::path www_root;

  std::optional<fs::path> state_file;
  std::optional<std::chrono::milliseconds> tick_period;
  std::optional<std::chrono::milliseconds> state_save_period;
};

enum class AppEnv : std::uint8_t {
  test,
  prod
};

struct AppConfig {
  AppEnv env { AppEnv::test };
  std::string_view log_level { "DEBUG"sv };
  
  mutable std::unique_ptr<model::Game> game;

  ServerConfig server;
};

void init(cli::Args args);

[[nodiscard]] const AppConfig& get() noexcept;

} // namespace config