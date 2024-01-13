#include "extra_data.hpp"

#include <stdexcept>

namespace extra_data {

using namespace std::literals;

const json::array& LootTypes::get(model::Map::Id map_id) const {
  if (!map_loot_types_.contains(map_id)) {
    throw std::invalid_argument("Can not get loot types. Invalid map id"s);
  }
  
  return map_loot_types_.at(map_id);
}

std::size_t LootTypes::loot_count(const model::Map::Id map_id) const {
  if (!map_loot_types_.contains(map_id)) {
    throw std::invalid_argument("Can not get loot types count. Invalid map id"s);
  }  
  
  return map_loot_types_.at(map_id).size();
}

void LootTypes::set(const model::Map::Id map_id, json::array loot_types) {
  map_loot_types_[map_id] = std::move(loot_types);
}

LootTypes& LootTypes::instance() noexcept {
  static LootTypes lt;
  return lt;
}

} // namespace extra_data 