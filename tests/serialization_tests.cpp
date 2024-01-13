#include "../src/serialization.hpp"

#include <sstream>
#include <boost/serialization/export.hpp>
#include <catch2/catch_test_macros.hpp>

// Для корректной (де-)сериализации абстрактного класса 
// и его наследников нужно объявить следующие макросы
BOOST_SERIALIZATION_ASSUME_ABSTRACT(model::Loot);
BOOST_CLASS_EXPORT(model::Wallet);
BOOST_CLASS_EXPORT(model::Key);

using namespace std::literals;
using namespace serialization;

namespace Catch {

template <>
struct StringMaker<geom::Position> {
  static std::string convert(const geom::Position& pos) {
    std::ostringstream ss;

    ss << "(x:" << pos.x << "; y:" << pos.y << ")";
    return ss.str();
  }
};

} // namespace Catch

namespace {

struct Fixture {
  std::stringstream ss;
  OutputArchive oa{ ss };
};

} // namespace

SCENARIO_METHOD(Fixture, "Point serialization") {
  GIVEN("A position") {
    const geom::Position pos {15.0, 20.0};

    WHEN("position is serialized") {
      oa << pos;

      THEN("it is equal to the position after deserialization") {
        InputArchive ia {ss};
        geom::Position restored_pos;

        ia >> restored_pos;
        CHECK(pos == restored_pos);
      }
    }
  }
}

SCENARIO_METHOD(Fixture, "Speed serialization") {
  GIVEN("A speed") {
    const geom::Speed speed {10.0, 10.0};

    WHEN("speed is serialized") {
      oa << speed;

      THEN("it is equal to the speed after deserialization") {
        InputArchive ia {ss};
        geom::Speed restored_speed;

        ia >> restored_speed;
        CHECK(speed == restored_speed);
      }
    }
  }
}

SCENARIO_METHOD(Fixture, "Dog serialization") {
  GIVEN("A dog") {
    constexpr auto BAGPACK_CAP = 3u;
    auto dog = model::create_character<model::Dog>("Tim"sv, BAGPACK_CAP);

    auto loot1 = model::create_loot("key"sv, 0, 5);
    loot1->position({1.0, -2.67});

    auto loot2 = model::create_loot("key"sv, 0, 5);
    loot2->position({-12.98, 4.11});

    auto loot3 = model::create_loot("key"sv, 0, 5);
    loot3->position({0.0, 0.0});

    auto loot4 = model::create_loot("key"sv, 0, 5); // Не должен быть добавлен
    loot4->position({22.13, -10.63});

    dog->bagpack.add(1, std::move(loot1));
    dog->bagpack.add(2, std::move(loot2));
    dog->bagpack.add(3, std::move(loot3));
    dog->bagpack.add(4, std::move(loot4));

    CHECK(dog->bagpack.size() == BAGPACK_CAP);

    dog->add_points(20u);
    dog->direction(model::Character::Direction::north);
    dog->speed({5.0, -1.0});
    dog->position({4.92, 17.0});

    WHEN("dog is serialized") {
      serialization::DogSerializer dogser(static_cast<const model::Dog&>(*dog));
      oa << dogser;

      THEN("it is equal to the dog after deserialization") {
        InputArchive ia {ss};
        serialization::DogSerializer dogser{};

        ia >> dogser;

        CHECK(dog->name() == dogser.name());
        CHECK(dog->bagpack.capacity() == dogser.bagpack().capacity());

        const auto& bench_bagpack = dog->bagpack.get();

        CHECK(bench_bagpack.size() == dogser.bagpack().size());

        for (const auto& [id, loot] : dogser.bagpack().get()) {   
          const auto it = bench_bagpack.find(id);

          CHECK(it != bench_bagpack.cend());
          CHECK(it->second->name() == loot->name());
          CHECK(it->second->position() == loot->position());
          CHECK(it->second->type() == loot->type());
          CHECK(it->second->value() == loot->value());
        }

        CHECK(dog->score() == dogser.score());
        CHECK(dog->direction().value() == dogser.direction().value());
        CHECK(dog->speed() == dogser.speed());
        CHECK(dog->position() == dogser.position());
      }
    }
  }  
}

SCENARIO_METHOD(Fixture, "Players serialization") {
  GIVEN("players") {
    // TODO
  }
}