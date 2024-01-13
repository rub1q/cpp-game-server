#pragma once 

#include "map.hpp"
#include "loot.hpp"

#include <string>
#include <memory>

namespace model {

class Bagpack {
public:
  using BagType = std::unordered_map<Loot::Id, std::shared_ptr<Loot>>;

  Bagpack() = default;

  explicit Bagpack(const std::size_t capacity) {
    bag_.reserve(capacity);
  }

  void add(Loot::Id id, std::shared_ptr<Loot> loot);
  void capacity(const std::uint64_t capacity);
  void clear() noexcept;
  
  [[nodiscard]] bool is_full() const noexcept;
  [[nodiscard]] const BagType& get() const noexcept;
  [[nodiscard]] std::size_t capacity() const noexcept;
  [[nodiscard]] std::size_t size() const noexcept;

private:
  BagType bag_;   
}; 

class Character {
public:
  using Id = std::uint64_t;
  using Points = std::uint64_t;

  class Direction final {
  public:
    friend class Character;
    
    enum Direct : std::uint8_t {
      nomove,
      north,
      south,
      west,
      east
    }; 

    Direction(const Direct direct)
      : direct_(direct) {
    }

    Direction(std::string_view letter_direct);

    [[nodiscard]] const std::string& as_letter() const noexcept;
    [[nodiscard]] Direct value() const noexcept;

  private:
    Direct direct_;
  };   

  Bagpack bagpack;

public:
  Character(const Character&) = delete;
  Character(Character&&) = default;

  Character& operator=(const Character&) = delete;
  Character& operator=(Character&&) = default;

  virtual ~Character() = default; 

  // Метод возвращает ширину конкретного персонажа
  // Возвращенное значение используется при рассчете коллизий 
  // во время перемещения персонажей и сбора предметов
  [[nodiscard]] virtual double width() const noexcept = 0;

  [[nodiscard]] std::string_view name() const noexcept;
  [[nodiscard]] const geom::Speed& speed() const noexcept;
  [[nodiscard]] const geom::Position& position() const noexcept;
  [[nodiscard]] const Direction& direction() const noexcept;
  [[nodiscard]] Points score() const noexcept;

  void name(std::string_view name) noexcept;
  void position(const geom::Position pos) noexcept;
  void move(Direction direction, const double speed) noexcept;
  void add_points(const Points points) noexcept;
  void speed(const geom::Speed speed);
  void direction(const Direction direction);

protected:
  Character() = default;

  explicit Character(std::string_view name, const std::uint64_t bagpack_capacity)
    : name_(std::move(name)) {
    
    bagpack.capacity(bagpack_capacity);
  }  

protected: 
  Points points_ { 0u };
  geom::Position position_; 
  geom::Speed speed_ {0.0, 0.0};
  Direction direction_ { Direction::north };

  std::string name_;
};

class Dog final : public Character {
public:
  explicit Dog(std::string_view name, const std::uint64_t bagpack_capacity) 
    : Character(std::move(name), bagpack_capacity) {
  }

  [[nodiscard]] double width() const noexcept override {
    return 0.6;
  }
};

template <typename ConcreteCharacter,
          typename... Args,
          std::enable_if_t<std::is_base_of_v<Character, ConcreteCharacter>, bool> = true>
inline std::shared_ptr<Character> create_character(std::string_view name, const std::uint64_t bagpack_capacity, Args&&... args) {
  return std::make_shared<ConcreteCharacter>(std::move(name), bagpack_capacity, std::forward<decltype(args)>(args)...);
} 

} // namespace model