#include "serialization.hpp"

namespace serialization {

void CharacterSerializer::serialize(InputArchive& ar, unsigned) {
  ar & name_;
  ar & pos_;
  ar & speed_;
  ar & direction_;
  ar & points_;
  ar & bagpack_; 
}

void CharacterSerializer::serialize(OutputArchive& ar, unsigned) {
  ar & name_;
  ar & pos_;
  ar & speed_;
  ar & direction_;
  ar & points_; 
  ar & bagpack_;
}

std::string_view CharacterSerializer::name() const noexcept {
  return name_;
}

const geom::Position& CharacterSerializer::position() const noexcept {
  return pos_;
}

const geom::Speed& CharacterSerializer::speed() const noexcept {
  return speed_;
}

const model::Character::Direction& CharacterSerializer::direction() const noexcept {
  return direction_;
}

model::Character::Points CharacterSerializer::score() const noexcept {
  return points_;
}

const model::Bagpack& CharacterSerializer::bagpack() const noexcept {
  return bagpack_;
}

void LootSerializer::serialize(InputArchive& ar, unsigned) {
  ar & name_;
  ar & pos_;
  ar & type_;
  ar & value_;
}

void LootSerializer::serialize(OutputArchive& ar, unsigned) {
  ar & name_;
  ar & pos_;
  ar & type_;
  ar & value_;
}

std::string_view LootSerializer::name() const noexcept {
  return name_;
}

const geom::Position& LootSerializer::position() const noexcept {
  return pos_;
}

model::Loot::Type LootSerializer::type() const noexcept {
  return type_;
}

model::Loot::Value LootSerializer::value() const noexcept {
  return value_;
}

} // namespace serialization