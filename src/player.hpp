#pragma once

#include "game.hpp"

#include <random>
#include <unordered_map>

namespace app {

struct Token {
  using Type = std::string; 

  virtual Token::Type get_new() noexcept = 0;
};

class PlayerToken final : public Token {
public:
  Token::Type get_new() noexcept override;

private:
  std::random_device random_device_ {};

  std::mt19937_64 random_generator_ {[this] {
    std::uniform_int_distribution<std::mt19937_64::result_type> dist;
    return dist(random_device_);
  }()};  

private:
  Token::Type sha256(std::string_view str);
};
 
class Player {
public:
  explicit Player(std::shared_ptr<model::Character> character, const model::GameSession& game_session)
    : character_(std::move(character))
    , game_session_(game_session) {
  }

  [[nodiscard]] const model::GameSession& game_session() const noexcept;
  [[nodiscard]] const model::Character& character() const noexcept;
  [[nodiscard]] model::Character& character() noexcept;

private:
  const model::GameSession& game_session_;
  std::shared_ptr<model::Character> character_;
};

class Players {
public:
  using PlayersByToken = std::unordered_map<Token::Type, std::unique_ptr<Player>>;
  using TokenPlayerPair = std::pair<Token::Type, Player*>;

  TokenPlayerPair new_player(std::shared_ptr<model::Character> character, const model::GameSession& session);
  TokenPlayerPair add_player(const Token::Type& token, std::unique_ptr<Player> player);
  Player* find_by_token(const Token::Type& token);

  [[nodiscard]] const PlayersByToken& all_players() const noexcept; 

  static Players& instance() noexcept;

private:
  Players() = default;

private:
  PlayersByToken players_by_token_;
};

} // namespace app