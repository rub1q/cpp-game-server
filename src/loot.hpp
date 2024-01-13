#pragma once 

#include "geometry.hpp"

#include <chrono>
#include <functional>
#include <memory>

namespace model {

class Loot {
public:
  using Id = std::uint64_t;
  using Type = std::uint64_t;
  using Value = std::uint64_t;

  Loot() = default;
  
  void position(const geom::Position position);
  void type(Type type) noexcept;
  void value(Value value) noexcept;
  void name(std::string_view name) noexcept;

  [[nodiscard]] const geom::Position& position() const noexcept;
  [[nodiscard]] Type type() const noexcept;
  [[nodiscard]] Value value() const noexcept;

  [[nodiscard]] virtual double width() const noexcept {
    return 0.0;
  }

  [[nodiscard]] std::string_view name() const noexcept;

protected: 
  explicit Loot(std::string_view name, Type type, Value value)
    : name_(name)
    , type_(type) 
    , value_(value) {
  }

protected:
  Type type_;
  Value value_;

  geom::Position position_;

  std::string name_;
};

class Key final : public Loot {
public:
  Key() = default;

  explicit Key(std::string_view name, Type type, Value value)
    : Loot(name, type, value) {
  }
};

class Wallet final : public Loot {
public:  
  Wallet() = default;

  explicit Wallet(std::string_view name, Type type, Value value)
    : Loot(name, type, value) {
  }
};

template <typename... Args>
inline std::shared_ptr<Loot> create_loot(std::string_view name, Loot::Type type, Loot::Value value, Args&&... args) {
  using namespace std::literals;
  
  if (name == "key"sv) {
    return std::make_shared<Key>(name, type, value);
  }
  if (name == "wallet"sv) {
    return std::make_shared<Wallet>(name, type, value);
  }

  return nullptr;
}

struct LootGeneratorConfig {
  // Вероятность появления на карте потерянного 
  // объекта в течение {period} секунд равна {probability}
  using TimeInterval = std::chrono::milliseconds;

  TimeInterval period;
  double probability;
};

class LootGenerator final {
public:
  using RandomGenerator = std::function<double()>;
  using TimeInterval = LootGeneratorConfig::TimeInterval;

  static LootGenerator& instance() noexcept;

  void config(LootGeneratorConfig cfg);
  void generator(RandomGenerator&& random_generator);

  /*
   * Возвращает количество трофеев, которые должны появиться на карте спустя
   * заданный промежуток времени.
   * Количество трофеев, появляющихся на карте не превышает количество мародёров.
   *
   * time_delta - отрезок времени, прошедший с момента предыдущего вызова generate
   * loot_count - количество трофеев на карте до вызова generate
   * looter_count - количество мародёров на карте
   */
  [[nodiscard]] unsigned generate(TimeInterval time_delta, unsigned loot_count, unsigned looter_count);

private:
  LootGenerator() = default;

private:
  static constexpr double default_generator() noexcept {
    return 1.0;
  } 

private: 
  TimeInterval time_without_loot_ {};

  LootGeneratorConfig cfg_;
  RandomGenerator random_generator_ { default_generator };
};


} // namespace model 