#include "core.hpp"

#include <random>

namespace core {

geom::Position GameEngine::generate_object_position(bool random_position) const {
  if (!random_position) {
    return { 0.0, 0.0 };
  }  

  decltype(auto) roads = map_.get_roads();  

  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<std::uint32_t> u_idx(0, roads.size()-1);
  
  const auto road_idx = u_idx(gen);
  decltype(auto) road = roads[road_idx];

  if (road.is_horizontal()) {
    std::uniform_real_distribution<double> ux(road.get_start().x, road.get_end().x);
    return { ux(gen), road.get_start().y };
  } else {
    std::uniform_real_distribution<double> uy(road.get_start().y, road.get_end().y);
    return { road.get_start().x, uy(gen) };    
  }
}

geom::Position GameEngine::calculate_object_position(const geom::Position& current_position, const geom::Speed& speed, std::int64_t delta) const {
  const auto road = map_.get_road_by_position(current_position);    
  
  if (!road) 
    return { 0.0, 0.0 };
  
  geom::Position new_position;

  new_position.x = current_position.x + (speed.x * delta / 1e3);
  new_position.y = current_position.y + (speed.y * delta / 1e3);

  if (road->is_horizontal()) {
    if (new_position.x > road->get_end().x) {
      new_position.x = road->get_end().x;
    } else if (new_position.x < road->get_start().x) {
      new_position.x = road->get_start().x;
    }

    if (new_position.y > (road->get_start().y + (model::Road::WIDTH / 2))) {
      new_position.y = road->get_start().y + (model::Road::WIDTH / 2);
    } else if (new_position.y < (road->get_start().y - (model::Road::WIDTH / 2))) {
      new_position.y = road->get_start().y - (model::Road::WIDTH / 2);
    }
  } else {
    if (new_position.y > road->get_end().y) {
      new_position.y = road->get_end().x;
    } else if (new_position.y < road->get_start().y) {
      new_position.y = road->get_start().y;
    }

    if (new_position.x > (road->get_start().x + (model::Road::WIDTH / 2))) {
      new_position.x = road->get_start().x + (model::Road::WIDTH / 2);
    } else if (new_position.x < (road->get_start().x - (model::Road::WIDTH / 2))) {
      new_position.x = road->get_start().x - (model::Road::WIDTH / 2);
    }  
  } 

  return new_position;
}

} // namespace core 