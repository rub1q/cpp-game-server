#include "character.hpp"
#include "config.hpp"

namespace model {

using namespace std::literals;

const std::unordered_map<Character::Direction::Direct, std::string> direct_to_letter_ = {
  { Character::Direction::nomove, "" },
  { Character::Direction::north,  "U" },
  { Character::Direction::south,  "D" },
  { Character::Direction::east,   "R" },
  { Character::Direction::west,   "L" },
};

const std::unordered_map<std::string, Character::Direction::Direct> letter_to_direct_ = {
  { "",  Character::Direction::nomove }, 
  { "U", Character::Direction::north },
  { "D", Character::Direction::south },
  { "R", Character::Direction::east },
  { "L", Character::Direction::west },
};

std::size_t Bagpack::capacity() const noexcept {
  return (bag_.max_load_factor() * bag_.bucket_count()); 
}

std::size_t Bagpack::size() const noexcept {
  return bag_.size();
}

void Bagpack::clear() noexcept {
  bag_.clear();
}

void Bagpack::add(Loot::Id id, std::shared_ptr<Loot> loot) {
  if (is_full()) {
    return;
  }

  bag_[id] = std::move(loot);
}

void Bagpack::capacity(const std::uint64_t capacity) {
  bag_.reserve(capacity);
}

bool Bagpack::is_full() const noexcept {
  const auto capacity = (bag_.max_load_factor() * bag_.bucket_count());    
  return (bag_.size() == capacity);
}

const Bagpack::BagType& Bagpack::get() const noexcept {
  return bag_;
}

std::string_view Character::name() const noexcept {
  return name_;
}

Character::Points Character::score() const noexcept {
  return points_;
}

void Character::add_points(const Points points) noexcept {
  points_ += points;
}

const geom::Speed& Character::speed() const noexcept {
  return speed_;
}

const geom::Position& Character::position() const noexcept {
  return position_;
}

const Character::Direction& Character::direction() const noexcept {
  return direction_;
}

Character::Direction::Direction(std::string_view letter_direct) {
  if (!letter_direct.empty()) {
    const auto pos = letter_direct.find_first_of("LRUD"sv);

    if (letter_direct.length() > 1 || (pos == std::string::npos)) {
      throw std::invalid_argument("Invalid direction letter"s);
    }
  }

  direct_ = letter_to_direct_.at(std::string(letter_direct));
}

Character::Direction::Direct Character::Direction::value() const noexcept {
  return direct_;
}

const std::string& Character::Direction::as_letter() const noexcept {
  return direct_to_letter_.at(direct_);
}

void Character::move(Direction direction, const double speed) noexcept {
  switch (direction.direct_) {
    case Direction::nomove :
      speed_ = {0.0, 0.0};
      break;
    case Direction::north :
      speed_ = {0.0, -speed};
      break;
    case Direction::south :
      speed_ = {0.0, speed};
      break;
    case Direction::east :
      speed_ = {speed, 0.0};
      break;
    case Direction::west :
      speed_ = {-speed, 0.0};
      break;
  }

  direction_ = std::move(direction);
}

void Character::name(std::string_view name) noexcept {
  name_ = std::string(name);
}

void Character::position(geom::Position pos) noexcept {
  position_ = std::move(pos);
}

void Character::speed(const geom::Speed speed) {
  speed_ = std::move(speed);
}

void Character::direction(const Character::Direction direction) {
  direction_ = std::move(direction);
}

} // namespace model 