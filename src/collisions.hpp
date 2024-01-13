#pragma once 

#include "geometry.hpp"
#include "loot.hpp"
#include "character.hpp"

#include <vector>

namespace collisions {

struct CollectionResult {
  bool is_collected(double collect_radius) const noexcept;

  // Квадрат расстояния до точки
  double sq_distance;
  // Доля пройденного отрезка
  double proj_ratio;
};

// Движемся из точки a в точку b и пытаемся подобрать точку c
CollectionResult try_collect_point(geom::Position a, geom::Position b, geom::Position c) noexcept;

enum class ObjectType {
  loot,
  base
};

struct Object {
  geom::Position position;
  double width;

public:
  Object() = default;

  Object(geom::Position pos, double w)
    : position(pos)
    , width(w) {
  }

  virtual ~Object() = default;
  virtual ObjectType type() const noexcept = 0;
}; 

struct Item final : public Object {
  model::Loot::Id id;

public:
  Item() = default;

  Item(geom::Position position, double width, model::Loot::Id id)
    : Object(position, width)
    , id(id) {
  }

  ObjectType type() const noexcept override {
    return ObjectType::loot;
  }
};

struct Base final : public Object {
  Base() = default;
  Base(geom::Position position, double width)
    : Object(position, width) {
  } 

  ObjectType type() const noexcept override {
    return ObjectType::base;
  }
};

struct Gatherer {
  geom::Position start_pos;
  geom::Position end_pos;

  double width;

  model::Character::Id id;
};

class ItemGathererProvider {
public:
  [[nodiscard]] virtual std::size_t objects_count() const noexcept = 0;
  [[nodiscard]] virtual const Object& get_object(const std::size_t idx) const = 0;
  [[nodiscard]] virtual std::size_t gatherers_count() const noexcept = 0;
  [[nodiscard]] virtual const Gatherer& get_gatherer(const std::size_t idx) const = 0;

protected:
  virtual ~ItemGathererProvider() = default;
};

class LootCharacterProvider final : public ItemGathererProvider {
public: 
  using Gatherers = std::vector<Gatherer>;
  using Objects = std::vector<std::unique_ptr<Object>>;
  
  [[nodiscard]] virtual std::size_t objects_count() const noexcept override;
  [[nodiscard]] virtual const Object& get_object(const std::size_t idx) const override;
  [[nodiscard]] virtual std::size_t gatherers_count() const noexcept override;
  [[nodiscard]] virtual const Gatherer& get_gatherer(const std::size_t idx) const override; 

  template <typename ConcreteObject, 
            std::enable_if_t<std::is_base_of_v<Object, ConcreteObject>, bool> = true>
  void add_object(ConcreteObject object) {
    objects_.emplace_back(std::make_unique<ConcreteObject>(std::move(object)));
  } 

  void add_gatherer(Gatherer gatherer); 

  void clear_objects() noexcept;
  void clear_gatherers() noexcept;

  static LootCharacterProvider& instance() noexcept;

private:
  LootCharacterProvider() = default;

private:
  Gatherers gatherers_;
  Objects objects_; 
}; 

struct GatheringEvent {
  std::size_t object_idx;
  std::size_t gatherer_idx;

  double sq_distance;
  double time;
};

using GatheringEvents = std::vector<GatheringEvent>;

// Функция возвращает вектор событий, идущих в хронологическом порядке
[[nodiscard]] GatheringEvents find_gather_events(const ItemGathererProvider& provider);

} // namespace collisions