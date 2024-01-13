#include "loot.hpp"

#include <algorithm>
#include <cmath>

namespace model {

std::string_view Loot::name() const noexcept {
  return name_;
}

void Loot::position(const geom::Position position) {
  position_ = std::move(position);
}

void Loot::type(Type type) noexcept {
  type_ = type;
}

void Loot::value(Value value) noexcept {
  value_ = value;
}

void Loot::name(std::string_view name) noexcept {
  name_ = name;
}

const geom::Position& Loot::position() const noexcept {
  return position_;
}

Loot::Type Loot::type() const noexcept {
  return type_;
}

Loot::Value Loot::value() const noexcept {
  return value_;
}

LootGenerator& LootGenerator::instance() noexcept {
  static LootGenerator lg;
  return lg;
}

void LootGenerator::config(LootGeneratorConfig cfg) {
  cfg_ = std::move(cfg);
}

void LootGenerator::generator(RandomGenerator&& random_generator) {
  random_generator_ = std::move(random_generator);
}

unsigned LootGenerator::generate(TimeInterval time_delta, unsigned loot_count, unsigned looter_count) {
  using namespace std::literals;
  
  time_without_loot_ += time_delta;

  const unsigned loot_shortage = loot_count > looter_count ? 0u : looter_count - loot_count;
  const double ratio = std::chrono::duration<double>{time_without_loot_} / cfg_.period;

  const double probability
    = std::clamp((1.0 - std::pow(1.0 - cfg_.probability, ratio)) * random_generator_(), 0.0, 1.0);

  const unsigned generated_loot = static_cast<unsigned>(std::round(loot_shortage * probability));
  
  if (generated_loot > 0) {
    time_without_loot_ = {};
  }

  return generated_loot;
}

} // namespace model