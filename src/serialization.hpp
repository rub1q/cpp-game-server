#pragma once

#include "geometry.hpp"
#include "loot.hpp"
#include "character.hpp"
#include "player.hpp"

#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace serialization {

using InputArchive = boost::archive::text_iarchive; 
using OutputArchive = boost::archive::text_oarchive;

/*
Состояние, которое сохраняется на диск, должно включать:
- информацию обо всех динамических объектах на всех картах — собаках и потерянных предметах;
- информацию о токенах и идентификаторах пользователей, вошедших в игру.
*/

class CharacterSerializer {
public:
  explicit CharacterSerializer(const model::Character& character)
    : name_(character.name())
    , pos_(character.position())
    , speed_(character.speed())
    , direction_(character.direction())
    , points_(character.score()) {

    bagpack_.clear();
    bagpack_.capacity(character.bagpack.capacity());

    for (const auto& [id, loot] : character.bagpack.get()) {   
      bagpack_.add(id, loot);
    }
  }

  virtual ~CharacterSerializer() = default;
  
  virtual void serialize(InputArchive& ar, unsigned);
  virtual void serialize(OutputArchive& ar, unsigned);

  [[nodiscard]] std::string_view name() const noexcept;
  [[nodiscard]] const geom::Position& position() const noexcept;
  [[nodiscard]] const geom::Speed& speed() const noexcept;
  [[nodiscard]] const model::Character::Direction& direction() const noexcept;
  [[nodiscard]] model::Character::Points score() const noexcept;
  [[nodiscard]] const model::Bagpack& bagpack() const noexcept;

protected:
  CharacterSerializer() = default;

protected:
  std::string name_;

  geom::Position pos_; 
  geom::Speed speed_;

  model::Character::Direction direction_ { model::Character::Direction::north };
  model::Character::Points points_;
  model::Bagpack bagpack_;
};

class DogSerializer final : public CharacterSerializer {
public:
  DogSerializer() = default;

  explicit DogSerializer(const model::Dog& dog) 
    : CharacterSerializer(dog) {
  }
};

class LootSerializer {
public:
  LootSerializer() = default;

  explicit LootSerializer(const model::Loot& loot)
    : name_(loot.name())
    , pos_(loot.position())
    , type_(loot.type())
    , value_(loot.value()) {
  }

  virtual void serialize(InputArchive& ar, unsigned);
  virtual void serialize(OutputArchive& ar, unsigned);

  [[nodiscard]] std::string_view name() const noexcept;
  [[nodiscard]] const geom::Position& position() const noexcept;
  [[nodiscard]] model::Loot::Type type() const noexcept;
  [[nodiscard]] model::Loot::Value value() const noexcept;

protected:
  std::string name_;
  geom::Position pos_;
  model::Loot::Type type_;
  model::Loot::Value value_;
};

} // namespace serialization

namespace geom {

template <typename Archive>
inline void serialize(Archive& ar, Position& pos, unsigned) {
  ar & pos.x;
  ar & pos.y;
}

template <typename Archive>
inline void serialize(Archive& ar, Speed& speed, unsigned) {
  ar & speed.x;
  ar & speed.y;
}

} // namespace geom 

namespace model {

// Character direction
inline void serialize(serialization::InputArchive& ar, model::Character::Direction& dir, unsigned) {
  model::Character::Direction::Direct direct;
  ar & direct;

  dir = direct;
}

inline void serialize(serialization::OutputArchive& ar, model::Character::Direction& dir, unsigned) {
  ar & dir.value();
}

// Bagpack
inline void serialize(serialization::InputArchive& ar, model::Bagpack& bagpack, unsigned) {  
  model::Bagpack::BagType bag;

  ar & bag;

  bagpack.clear();
  bagpack.capacity(bag.size());

  for (auto& [id, loot] : bag) {
    bagpack.add(id, std::move(loot));
  }
}

inline void serialize(serialization::OutputArchive& ar, model::Bagpack& bagpack, unsigned) {
  ar & bagpack.get();
}

// Loot
inline void serialize(serialization::InputArchive& ar, model::Loot& loot, unsigned) {
  serialization::LootSerializer ser_loot;
  ar & ser_loot;

  loot.name(ser_loot.name());
  loot.position(ser_loot.position()); 
  loot.type(ser_loot.type());
  loot.value(ser_loot.value());
}

inline void serialize(serialization::OutputArchive& ar, model::Loot& loot, unsigned) {
  ar & serialization::LootSerializer(loot);
}

template <typename Archive>
inline void serialize(Archive& ar, model::Key& key, unsigned) {
  ar & boost::serialization::base_object<model::Loot>(key);
}

template <typename Archive>
inline void serialize(Archive& ar, model::Wallet& wallet, unsigned) {
  ar & boost::serialization::base_object<model::Loot>(wallet);
} 

} // namespace model 