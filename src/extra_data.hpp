#pragma once 

#include "map.hpp"
#include <boost/json.hpp>

// This namespace stores data that can't be included into model
namespace extra_data {

namespace json = boost::json;

class LootTypes final {
public:
  void set(const model::Map::Id map_id, json::array loot_types);

  [[nodiscard]] const json::array& get(const model::Map::Id map_id) const;
  [[nodiscard]] std::size_t loot_count(const model::Map::Id map_id) const;

  static LootTypes& instance() noexcept;

private:
  LootTypes() = default;

private:
  using MapLootTypes = std::unordered_map<model::Map::Id, json::array, model::Map::IdHasher>;
  MapLootTypes map_loot_types_;
};

} // namespace extra_data 