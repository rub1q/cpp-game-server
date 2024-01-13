#pragma once

#include "game.hpp"

#include <filesystem>

namespace json_loader {

[[nodiscard]] model::Game load_game(const std::filesystem::path& config_path, bool randomize_spawn);

}  // namespace json_loader
