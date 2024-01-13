#include "config.hpp"
#include "json_loader.hpp"

#include <mutex>

namespace config {

namespace { 
  std::once_flag once_flag;
  AppConfig cfg;
} // namespace

const AppConfig& get() noexcept {
  return cfg;
}

void init(cli::Args args) {
  std::call_once(once_flag, [args = std::move(args)] {
    if (args.config_file.empty()) {
      throw std::invalid_argument("Invalid map config path"s);
    }

    if (args.www_root.empty()) {
      throw std::invalid_argument("Invalid root path"s);
    }  

    cfg.server.www_root = std::move(args.www_root);    

    if (args.tick_period.has_value()) {
      cfg.env = AppEnv::prod;
      cfg.server.tick_period = std::chrono::milliseconds(*args.tick_period);
    }

    if (args.save_state_period.has_value()) {
      cfg.server.state_save_period = std::chrono::milliseconds(*args.save_state_period);
    }

    if (args.state_file.has_value()) {
      cfg.server.state_file = std::move(*args.state_file);
    }

    cfg.game = std::make_unique<model::Game>(json_loader::load_game(std::move(args.config_file), args.randomize_spawn));

    if (const auto addr = std::getenv("GAME_SERVER_HTTP_ADDR")) {
      cfg.server.addr = net::ip::make_address(addr);
    }

    if (const auto port = std::getenv("GAME_SERVER_HTTP_PORT")) {
      char* end = nullptr;
      const auto p = std::strtoul(port, &end, 10);

      if (errno != ERANGE && p) {
        cfg.server.port = p;
      }
    }  

    if (const auto log_level = std::getenv("GAME_SERVER_LOG_LEVEL")) {
      cfg.log_level = log_level;
    }
  });
}

} // namespace config