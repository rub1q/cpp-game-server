#pragma once 

#include <cstdint>  
#include <functional>

namespace geom {

using Dimension = std::int64_t;
using Coord = double;

struct Position {
  Coord x, y;

  auto operator<=>(const Position&) const = default;
};

struct PositionHasher {
  std::size_t operator()(const Position& pos) const noexcept {
    // https://stackoverflow.com/a/1646913/19932503 
    std::size_t res = 17;
    res = res * 31 + std::hash<double>{}(pos.x);
    res = res * 31 + std::hash<double>{}(pos.y);

    return res;
  }
};

struct Size {
  Dimension width, height;
};

struct Rectangle {
  Position position;
  Size size;
};

struct Offset {
  Dimension dx, dy;
};

struct Speed {
  Coord x, y;

  auto operator<=>(const Speed&) const = default;
};

} // namespace geom 