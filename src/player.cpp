#include "player.hpp"
#include "logger.hpp"
#include "openssl/sha.h"

namespace app {

using namespace std::literals;

Token::Type PlayerToken::get_new() noexcept {
  std::stringstream token;
  
  try {
    token << sha256(std::to_string(random_generator_())).substr(0, 32);
  } catch (const std::exception& e) {
    LOG_ERROR << JSON_DATA({
      {"exception"s, e.what()},
      {"where"s, __FUNCTION__}
    }) 
    << "error"sv; 

    token << std::hex << random_generator_() << random_generator_();
  }

  return token.str();
}

Token::Type PlayerToken::sha256(std::string_view str) {
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;

  if (!SHA256_Init(&sha256))
    throw std::runtime_error("SHA256_Init error"s);

  if (!SHA256_Update(&sha256, str.data(), str.size()))
    throw std::runtime_error("SHA1_Update error"s);

  if (!SHA256_Final(hash, &sha256))
    throw std::runtime_error("SHA256_Final error"s);

  std::stringstream ss;

  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(hash[i]);
  }

  return ss.str();
}

const model::GameSession& Player::game_session() const noexcept {
  return game_session_;
}

const model::Character& Player::character() const noexcept {
  return *character_.get();
}

model::Character& Player::character() noexcept {
  return *character_.get();
}

Players::TokenPlayerPair Players::new_player(std::shared_ptr<model::Character> character, const model::GameSession& session) {
  const auto it = players_by_token_.emplace(PlayerToken().get_new(), std::make_unique<Player>(character, session));
  return { it.first->first, it.first->second.get() };
}

Players::TokenPlayerPair Players::add_player(const Token::Type& token, std::unique_ptr<Player> player) {
  const auto it = players_by_token_.emplace(std::move(token), std::move(player));
  return { it.first->first, it.first->second.get() };
}

Player* Players::find_by_token(const Token::Type& token) {
  const auto it = players_by_token_.find(token);
  if (it != players_by_token_.cend())
    return it->second.get();

  return nullptr;
}

Players& Players::instance() noexcept {
  static Players p;
  return p;
}

const Players::PlayersByToken& Players::all_players() const noexcept {
  return players_by_token_;
}

} // namespace app