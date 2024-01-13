#pragma once 

#include <optional>
#include <filesystem>

namespace cli {

namespace fs = std::filesystem;

struct Args {
  bool randomize_spawn;

  std::optional<std::size_t> tick_period { std::nullopt };
  std::optional<std::size_t> save_state_period { std::nullopt };
  std::optional<fs::path> state_file { std::nullopt };

  fs::path config_file;
  fs::path www_root;
};

[[nodiscard]] std::optional<Args> parse(int argc, char** const argv);

} // namespace cli