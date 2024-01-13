#include "map.hpp"

#include <stdexcept>
#include <cmath>

namespace model {

using namespace std::literals;

bool Road::is_horizontal() const noexcept {
  return start_.y == end_.y;
}

bool Road::is_vertical() const noexcept {
  return start_.x == end_.x;
}

geom::Position Road::get_start() const noexcept {
  return start_;
}

geom::Position Road::get_end() const noexcept {
  return end_;
}

const geom::Rectangle& Building::get_bounds() const noexcept {
  return bounds_;
}

const Office::Id& Office::get_id() const noexcept {
  return id_;
}

geom::Position Office::get_position() const noexcept {
  return position_;
}

geom::Offset Office::get_offset() const noexcept {
  return offset_;
}

const Map::Id& Map::get_id() const noexcept {
  return id_;
}

const std::string& Map::get_name() const noexcept {
  return name_;
}

const Map::Buildings& Map::get_buildings() const noexcept {
  return buildings_;
}

const Map::Roads& Map::get_roads() const noexcept {
  return roads_;
}

const Map::Offices& Map::get_offices() const noexcept {
  return offices_;
}

void Map::add_road_position(const Road& road) {
  geom::Coord start, start_x, start_y, end;    

  if (road.is_horizontal()) {
    start_x = road.get_start().x;
    start_y = road.get_start().y;
    
    start = start_x;
    end = road.get_end().x;
  } else {
    start_x = road.get_start().x;
    start_y = road.get_start().y;

    start = start_y;
    end = road.get_end().y;
  }

  if (start > end) {
    std::swap(start, end);
  }

  for (auto i = start; i <= end; i++) {
    if (road.is_horizontal())
      roads_positions[{i, start_y}] = road; 
    else
      roads_positions[{start_x, i}] = road; 
  }
}

void Map::add_road(const Road& road) {
  add_road_position(road);
  roads_.emplace_back(road);
}

void Map::add_building(const Building& building) {
  buildings_.emplace_back(building);
}

void Map::add_office(const Office& office) {
  if (warehouse_id_to_index_.contains(office.get_id())) {
    throw std::invalid_argument("Duplicate warehouse"s);
  }

  const size_t index = offices_.size();
  
  Office& o = offices_.emplace_back(std::move(office));
  try {
    warehouse_id_to_index_.emplace(o.get_id(), index);
  } catch (...) {
    // Удаляем офис из вектора, если не удалось вставить в unordered_map
    offices_.pop_back();
    throw;
  }
}

const Road* Map::get_road_by_position(const geom::Position& position) const {
  const auto it = roads_positions.find({std::round(position.x), std::round(position.y)});
  if (it != roads_positions.cend()) 
    return &it->second;

  return nullptr;
}

}  // namespace model
