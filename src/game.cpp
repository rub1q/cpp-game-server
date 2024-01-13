#include "game.hpp"
#include "extra_data.hpp"
#include "collisions.hpp"

#include <numeric>
#include <stdexcept>
#include <random>

namespace model {

using namespace std::literals;

GameSession* GameSessionManager::get_session(const Game& game, const Map* map) {   
  if (!map || !game.find_map(map->get_id())) {
    throw std::invalid_argument("Invalid map"s);
  }

  auto sessions = sessions_.equal_range(map->get_id()); 

  std::uint16_t min_players = std::numeric_limits<std::uint16_t>::max();
  GameSession* relevant_session = nullptr;

  // Выбираем игровую сессию с минимальным кол-вом игроков на карте
  for (auto it = sessions.first; it != sessions.second; it++) {
    const auto players_count = it->second->characters_count();

    if (players_count == it->second->config().max_players)
      continue;
    
    if (players_count < min_players) {
      min_players = players_count;
      relevant_session = it->second.get();
    }
  }

  if (!relevant_session) {
    GameSessionConfig cfg;
    const auto& game_cfg = game.config();

    cfg.randomize_spawn = game_cfg.randomize_spawn;

    if (const auto it = game_cfg.map_character_speed.find(map->get_id()); 
        it != game_cfg.map_character_speed.cend()) {
      cfg.characters_speed = it->second;
    }

    if (const auto it = game_cfg.map_bag_capacity.find(map->get_id()); 
        it != game_cfg.map_bag_capacity.cend()) {
      cfg.bag_capacity = it->second;
    }

    relevant_session = create_session(std::move(cfg), *map); 
  }

  return relevant_session;
}

const GameSession* GameSessionManager::get_session(const Game& game, const Map* map) const { 
  return get_session(game, map);
}

const GameSessionManager::Sessions& GameSessionManager::all_sessions() const noexcept {
  return sessions_;
}

GameSession* GameSessionManager::create_session(GameSessionConfig cfg, const Map& map) {
  const auto it = sessions_.emplace(map.get_id(), std::make_unique<GameSession>(std::move(cfg), map));  
  return it->second.get();
}

void Game::add_map(const Map& map) {
  const std::size_t index = maps_.size();

  auto [it, inserted] = map_id_to_index_.emplace(map.get_id(), index);

  if (!inserted) {
    throw std::invalid_argument("Map with id "s + *map.get_id() + " already exists"s);
  } 

  try {
    maps_.emplace_back(std::move(map));
  } catch (...) {
    map_id_to_index_.erase(it);
    throw;
  }
}

const Game::Maps& Game::get_maps() const noexcept {
  return maps_;
}

const Map* Game::find_map(const Map::Id& id) const noexcept {
  if (const auto it = map_id_to_index_.find(id); it != map_id_to_index_.cend()) {
    return &maps_.at(it->second);
  }

  return nullptr;
}

const GameConfig& Game::config() const noexcept {
  return cfg_;
}

void Game::config(GameConfig cfg) {
  cfg_ = std::move(cfg);
}

void Game::refresh_state(std::int64_t delta) {
  if (!session_manager) {
    return; 
  }
  
  for (const auto& [_, session] : session_manager->all_sessions()) {
    session->recalc_characters_position(delta);
    session->spawn_lost_objects(delta); 

    session->process_collisions();
  }
}

const GameSession* Game::get_session(const Map* map) const {
  return get_session(map);
}

GameSession* Game::get_session(const Map* map) {
  if (!session_manager) {
    session_manager = std::make_unique<GameSessionManager>();
  }

  return session_manager->get_session(*this, map);
}

const Map& GameSession::map() const noexcept {
  return map_;
}

void GameSession::process_collisions() {
  auto& provider = collisions::LootCharacterProvider::instance();
  
  for (const auto& office : map_.get_offices()) {
    provider.add_object<collisions::Base>({office.get_position(), office.WIDTH});  
  } 
   
  const auto events = collisions::find_gather_events(provider);

  // 1. Игрок берёт все предметы, мимо которых он проходит, если рюкзак не полон
  // 2. Игрок пропускает предмет, если рюкзак полон
  // 3. Проходя мимо базы, игрок убирает все предметы из рюкзака.

  for (const auto& event : events) {
    const auto& gatherer = provider.get_gatherer(event.gatherer_idx);
    auto it_char = characters_.find(gatherer.id);
    
    if (it_char == characters_.end()) {
      continue;
    }

    auto character = it_char->second.get();
    const auto& object = provider.get_object(event.object_idx);
    
    switch (object.type()) {
      case collisions::ObjectType::loot : {
        auto item = static_cast<const collisions::Item&>(object);
        
        auto it_loot = lost_objects_.find(item.id); 

        if (!character->bagpack.is_full()) {
          // Перемещаем предмент в рюкзак игрока
          character->bagpack.add(it_loot->first, it_loot->second);
          // Убираем предмет с карты
          lost_objects_.erase(it_loot);
        }

        break;
      } case collisions::ObjectType::base : {
        if (character->bagpack.is_full()) {
          for (const auto& [_, loot] : character->bagpack.get()) {
            // Начисляем очки игроку за каждый собранный предмет
            character->add_points(loot->value());
          }

          character->bagpack.clear();
        }

        break;        
      }
    }
  } 

  provider.clear_gatherers();
  provider.clear_objects();
}

GameSession::IdCharPair GameSession::add_character(std::shared_ptr<Character> character) {  
  static const bool randomize_spawn = cfg_.randomize_spawn;
  const auto pos = engine.generate_object_position(randomize_spawn); 

  character->position(std::move(pos));
  
  const auto [it, _] = characters_.try_emplace(character_id_++, std::move(character));
  return { it->first, it->second };
}

GameSession::IdLootPair GameSession::add_lost_object(std::shared_ptr<Loot> lost_object) {
  const auto pos = engine.generate_object_position(false);
  
  lost_object->position(std::move(pos));

  const auto [it, _] = lost_objects_.try_emplace(loot_id_++, std::move(lost_object));
  return { it->first, it->second }; 
}

[[nodiscard]] std::size_t GameSession::characters_count() const noexcept {
  return characters_.size();
}

const GameSession::Characters& GameSession::characters() const noexcept {
  return characters_;
}

const GameSession::LostObjects& GameSession::lost_objects() const noexcept {
  return lost_objects_;
}

void GameSession::recalc_characters_position(std::int64_t delta) const {
  for (auto& [id, character] : characters_) {
    auto new_position = engine.calculate_object_position(character->position(), character->speed(), delta);

    collisions::Gatherer gatherer {
      .start_pos = character->position(),
      .end_pos = new_position,
      .width = character->width(),
      .id = id,
    };

    collisions::LootCharacterProvider::instance().add_gatherer(std::move(gatherer));

    character->position(std::move(new_position));
  }
}

void GameSession::spawn_lost_objects(std::int64_t delta) { 
  const auto lost_loot_amount = 
    model::LootGenerator::instance().generate(std::chrono::milliseconds(delta), lost_objects_.size(), characters_.size());   
  
  if (lost_loot_amount == 0)
    return; // nothing to spawn

  std::random_device rd;
  std::mt19937_64 gen(rd());

  auto& loot_types = extra_data::LootTypes::instance(); 
  
  const auto loot_count = loot_types.loot_count(map_.get_id());
  std::uniform_int_distribution<std::uint32_t> u_idx(0, loot_count-1); 

  for (std::size_t i = 0; i < lost_loot_amount; i++) {
    const auto loot_type = u_idx(gen);
    auto& loot = loot_types.get(map_.get_id())[loot_type];

    const auto loot_name = loot.at("name"sv).as_string().c_str();
    const auto loot_value = loot.at("value"sv).as_int64();

    auto lost_object = create_loot(loot_name, loot_type, loot_value);

    if (lost_object) {
      const auto [id, obj] = add_lost_object(std::move(lost_object)); 

      collisions::Item item(obj->position(), obj->width(), id);     
      collisions::LootCharacterProvider::instance().add_object(std::move(item));
    }
  }
}

const GameSessionConfig& GameSession::config() const noexcept {
  return cfg_;
}

} // namespace model