#pragma once 

#include "map.hpp"
#include "geometry.hpp"

namespace core {

class GameEngine final {
public:
  explicit GameEngine(const model::Map& map)
    : map_(map) {
  }

  [[nodiscard]] geom::Position generate_object_position(bool random_position = true) const;
  [[nodiscard]] geom::Position calculate_object_position(const geom::Position& current_position, const geom::Speed& speed, std::int64_t delta) const; 

private:
  const model::Map& map_;
};
   
} // namespace core