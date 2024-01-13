#pragma once

#include "character.hpp"
#include "map.hpp"
#include "loot.hpp"
#include "core.hpp"

namespace model {

struct GameConfig {
  using MapCharacterSpeed = std::unordered_map<Map::Id, double, Map::IdHasher>;
  using MapBagCapacity = std::unordered_map<Map::Id, std::uint64_t, Map::IdHasher>;
  using MapMaxPlayers = std::unordered_map<Map::Id, std::uint16_t, Map::IdHasher>;

  bool randomize_spawn;

  MapCharacterSpeed map_character_speed;
  MapBagCapacity map_bag_capacity;
  MapMaxPlayers map_max_players;
};

struct GameSessionConfig {
  bool randomize_spawn;
  
  std::uint16_t max_players { 8u };
  std::uint64_t bag_capacity { 3u };

  double characters_speed;
};

class GameSession final {
public: 
  using IdCharPair = std::pair<Character::Id, std::shared_ptr<Character>>;
  using IdLootPair = std::pair<Loot::Id, std::shared_ptr<Loot>>;

  using Characters = std::unordered_map<Character::Id, std::shared_ptr<Character>>;
  using LostObjects = std::unordered_map<Loot::Id, std::shared_ptr<Loot>>;

  explicit GameSession(GameSessionConfig config, const Map& map)
    : cfg_(std::move(config))
    , map_(map) {

    characters_.reserve(cfg_.max_players);
  }
 
  GameSession(const GameSession&) = delete;
  GameSession(GameSession&&) = default;

  GameSession& operator=(const GameSession&) = delete;
  GameSession& operator=(GameSession&&) = default;

  IdCharPair add_character(std::shared_ptr<Character> character);
  IdLootPair add_lost_object(std::shared_ptr<Loot> lost_object);

  [[nodiscard]] const Characters& characters() const noexcept;
  [[nodiscard]] const LostObjects& lost_objects() const noexcept;

  [[nodiscard]] const GameSessionConfig& config() const noexcept;
  [[nodiscard]] const Map& map() const noexcept;

  [[nodiscard]] std::size_t characters_count() const noexcept;

  void recalc_characters_position(std::int64_t delta) const;
  void spawn_lost_objects(std::int64_t delta);
  void process_collisions();

private:
  Character::Id character_id_ { 1u };
  Loot::Id loot_id_ { 1u };
  
  Characters characters_;
  LostObjects lost_objects_;

  const Map& map_;
  GameSessionConfig cfg_;

  core::GameEngine engine { map_ };
};

class Game;

class GameSessionManager {
public:
  using Sessions = std::unordered_multimap<Map::Id, std::unique_ptr<GameSession>, Map::IdHasher>;

  [[nodiscard]] const GameSession* get_session(const Game& game, const Map* map) const; 
  [[nodiscard]] GameSession* get_session(const Game& game, const Map* map);

  [[nodiscard]] const Sessions& all_sessions() const noexcept;

private:
  [[nodiscard]] GameSession* create_session(GameSessionConfig cfg, const Map& map);

private:
  Sessions sessions_;
};

class Game { 
public:
  using Maps = std::vector<Map>;

public:
  Game() = default;

  explicit Game(const GameConfig& config)
    : cfg_(std::move(config)) {
  }

  Game(const Game&) = delete;
  Game(Game&&) = default;

  Game& operator=(const Game&) = delete;
  Game& operator=(Game&&) = default;

  void add_map(const Map& map);
  void config(GameConfig config);
  void refresh_state(std::int64_t delta);

  const Map* find_map(const Map::Id& id) const noexcept;
  [[nodiscard]] const Maps& get_maps() const noexcept;
  [[nodiscard]] const GameConfig& config() const noexcept;

  [[nodiscard]] const GameSession* get_session(const Map* map) const; 
  [[nodiscard]] GameSession* get_session(const Map* map);

private:
  using MapIdToIndex = std::unordered_map<Map::Id, std::size_t, Map::IdHasher>;

  GameConfig cfg_;
  Maps maps_;

  std::unique_ptr<GameSessionManager> session_manager;

  MapIdToIndex map_id_to_index_;
};

} // namespace model 