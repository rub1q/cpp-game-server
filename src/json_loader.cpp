#include "json_loader.hpp"
#include "extra_data.hpp"

#include <fstream>
#include <boost/json/src.hpp>

namespace json_loader {

namespace json = boost::json;

using namespace std::literals;

namespace {

struct MapInstanceLoader {
public:
  virtual void load(const json::value& val) = 0;

protected:
  MapInstanceLoader(model::Map& map)
    : map_(map) {
  }

  model::Map& map_;
};

struct RoadLoader : public MapInstanceLoader {
public:
  RoadLoader(model::Map& map)
    : MapInstanceLoader(map) {
  }
  
  void load(const json::value& road) override {
    const geom::Coord x0 = road.at("x0"sv).as_int64();
    const geom::Coord y0 = road.at("y0"sv).as_int64();

    try {
      geom::Coord end_x = road.at("x1"sv).as_int64();
      map_.add_road({ model::Road::HORIZONTAL, {x0, y0}, end_x });
    } catch (const std::out_of_range& e) {
      geom::Coord end_y = road.at("y1"sv).as_int64();
      map_.add_road({ model::Road::VERTICAL, {x0, y0}, end_y });
    } 
  }
};

struct BuildingLoader : public MapInstanceLoader {
public:
  BuildingLoader(model::Map& map)
    : MapInstanceLoader(map) {
  }
  
  void load(const json::value& building) override {
    const geom::Coord x = building.at("x"sv).as_int64(); 
    const geom::Coord y = building.at("x"sv).as_int64(); 

    const auto w = building.at("w"sv).as_int64(); 
    const auto h = building.at("h"sv).as_int64();
    
    geom::Position point {.x = x, .y = y};
    geom::Size size {.width = w, .height = h};
    geom::Rectangle rect {point, size};
    
    map_.add_building(model::Building(rect));
  }
};

struct OfficeLoader : public MapInstanceLoader {
public:
  OfficeLoader(model::Map& map)
    : MapInstanceLoader(map) {
  }
  
  void load(const json::value& office) override {
    model::Office::Id id { office.at("id"sv).as_string().c_str() };

    const geom::Coord x = office.at("x"sv).as_int64();
    const geom::Coord y = office.at("y"sv).as_int64();

    const auto offsetX = office.at("offsetX"sv).as_int64();
    const auto offsetY = office.at("offsetY"sv).as_int64();

    geom::Position point {.x = x, .y = y};
    geom::Offset offset {.dx = offsetX, .dy = offsetY };
    
    map_.add_office(model::Office(id, point, offset));
  }
};

void set_loot_gen_config(const json::object& obj) {
  decltype(auto) loot_gen_config_obj = obj.at("lootGeneratorConfig"sv).as_object();

  const auto period = std::chrono::duration<double>(loot_gen_config_obj.at("period"sv).as_double());
  const auto probability = loot_gen_config_obj.at("probability"sv).as_double();

  model::LootGeneratorConfig loot_gen_config {
    .period = std::chrono::duration_cast<model::LootGenerator::TimeInterval>(period), 
    .probability = probability
  };

  model::LootGenerator::instance().config(std::move(loot_gen_config));
}

void set_loot_types(const json::value& val, model::Map::Id id) {
  decltype(auto) loot_types = val.at("lootTypes"sv).as_array();
  
  // В валидном конфигурационном файле должно содержаться не менее одного типа трофеев
  if (loot_types.size() < 1u) {
    throw std::invalid_argument("'lootTypes' must have minimum 1 trophy"s);
  }

  extra_data::LootTypes::instance().set(id, std::move(loot_types));
}

} // namespace

model::Game load_game(const std::filesystem::path& config_path, bool randomize_spawn) {
  std::ifstream jsonfile(config_path);

  if (!jsonfile) { 
    throw std::runtime_error("Unable to open the config file"s);
  }

  std::string input {std::istreambuf_iterator<char>(jsonfile), {}};
  jsonfile.close();

  const auto obj = json::parse(input).as_object();
  decltype(auto) maps = obj.at("maps"sv).as_array();

  model::GameConfig cfg;
  cfg.randomize_spawn = randomize_spawn;

  double default_speed = 1.0;
 
  if (const auto it = obj.find("defaultDogSpeed"sv); it != obj.cend()) {
    default_speed = it->value().as_double();
  }

  std::uint64_t default_bag_capacity = 3u;

  if (const auto it = obj.find("defaultBagCapacity"sv); it != obj.cend()) {
    default_bag_capacity = it->value().as_int64();
  }

  std::uint16_t default_max_players = 8u;

  if (const auto it = obj.find("defaultMaxPlayers"sv); it != obj.cend()) {
    default_max_players = it->value().as_int64();
  }

  set_loot_gen_config(obj);

  model::Game game;

  for (const auto& m : maps) {
    const model::Map::Id id { m.at("id"sv).as_string().c_str() };
    model::Map map { id, m.at("name"sv).as_string().c_str() };

    try {
      cfg.map_character_speed[id] = m.at("dogSpeed"sv).as_double();
    } catch (const std::out_of_range& e) {
      cfg.map_character_speed[id] = default_speed;
    }

    try {
      cfg.map_bag_capacity[id] = m.at("bagCapacity").as_int64();
    } catch (const std::out_of_range& e) {
      cfg.map_bag_capacity[id] = default_bag_capacity;
    }

    set_loot_types(m, id);

    cfg.map_max_players[id] = default_max_players;

    RoadLoader rl(map);
    BuildingLoader bl(map);
    OfficeLoader ol(map);

    for (const auto& v : m.at("roads"sv).as_array()) {
      rl.load(v);
    }

    for (const auto& v : m.at("buildings"sv).as_array()) {
      bl.load(v);
    }

    for (const auto& v : m.at("offices"sv).as_array()) {
      ol.load(v);
    }

    game.add_map(std::move(map));
  }

  game.config(std::move(cfg));
  
  return game;
}

}  // namespace json_loader
