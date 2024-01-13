#include "../src/collisions.hpp"

#include <cmath>
#include <sstream>
#include <algorithm>
#include <memory>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

namespace Catch {

template<>
struct StringMaker<collisions::GatheringEvent> {
  static std::string convert(const collisions::GatheringEvent& value) {
    std::ostringstream ss;

    ss << '(' << value.gatherer_idx 
       << ',' << value.object_idx
       << ',' << value.sq_distance 
       << ',' << value.time << ')';

    return ss.str();
  }
};

} // namespace Catch

namespace {

template <typename Range, typename Predicate>
class EqualsRangeMatcher : public Catch::Matchers::MatcherGenericBase {
public:  
  EqualsRangeMatcher(const Range& range, Predicate&& predicate)
    : range_(range)
    , predicate_(predicate) {
  }

  template <typename OtherRange>
  bool match(const OtherRange& other) const {
    return std::equal(std::begin(range_), std::end(range_), 
                      std::begin(other), std::end(other), 
                      predicate_);
  }

  std::string describe() const override {
    using namespace std::literals;
    return "Equals"s.append(Catch::rangeToString(range_));
  }

private:
  const Range& range_;
  Predicate predicate_;
};

struct CompareEvents {
  bool operator()(const collisions::GatheringEvent& l, const collisions::GatheringEvent& r) const {
    if ((l.gatherer_idx != r.gatherer_idx) || (l.object_idx != r.object_idx)) { 
      return false;
    }

    static constexpr double eps = 1e-10;

    if (std::abs(l.sq_distance - r.sq_distance) > eps) {
      return false;
    }

    if (std::abs(l.time - r.time) > eps) {
      return false;
    }

    return true;
  }
};

} // namespace

SCENARIO("Collisions detect") {
  WHEN("Empty items") {
    auto& provider = collisions::LootCharacterProvider::instance();

    provider.add_gatherer({{1, 2}, {4, 2}, 5.0, 0});
    provider.add_gatherer({{0, 0}, {10, 10}, 5.0, 1});
    provider.add_gatherer({{-5, 0}, {10, 5}, 5.0, 2});

    THEN("No events") {
      const auto events = collisions::find_gather_events(provider);
      CHECK(events.empty());
    }

    provider.clear_gatherers();
  }

  WHEN("Empty gatherers") {
    auto& provider = collisions::LootCharacterProvider::instance();

    provider.add_object<collisions::Item>({{1, 2}, 5.0, 0});
    provider.add_object<collisions::Item>({{0, 0}, 5.0, 1});
    provider.add_object<collisions::Item>({{-5, 0}, 5.0, 2});

    THEN("No events") {
      const auto events = collisions::find_gather_events(provider);
      CHECK(events.empty());
    }

    provider.clear_objects();
  }

  WHEN("Multiple items") {
    auto& provider = collisions::LootCharacterProvider::instance();

    provider.add_object<collisions::Item>({{9, 0.27}, 0.1, 0});
    provider.add_object<collisions::Item>({{8, 0.24}, 0.1, 1});
    provider.add_object<collisions::Item>({{7, 0.21}, 0.1, 2});
    provider.add_object<collisions::Item>({{6, 0.18}, 0.1, 3});
    provider.add_object<collisions::Item>({{5, 0.15}, 0.1, 4});
    provider.add_object<collisions::Item>({{4, 0.12}, 0.1, 5});
    provider.add_object<collisions::Item>({{3, 0.09}, 0.1, 6});
    provider.add_object<collisions::Item>({{2, 0.06}, 0.1, 7});
    provider.add_object<collisions::Item>({{1, 0.03}, 0.1, 8});
    provider.add_object<collisions::Item>({{0, 0.0}, 0.1, 9});
    provider.add_object<collisions::Item>({{-1, 0}, 0.1, 10});

    provider.add_gatherer({{0, 0}, {10, 0}, 0.1, 0});

    THEN("Items store in right order") {
      const auto events = collisions::find_gather_events(provider);

      CHECK_THAT(events, 
                  EqualsRangeMatcher(std::vector{
                    collisions::GatheringEvent{9, 0, 0.0*0.0, 0.0},
                    collisions::GatheringEvent{8, 0, 0.03*0.03, 0.1},
                    collisions::GatheringEvent{7, 0, 0.06*0.06, 0.2},
                    collisions::GatheringEvent{6, 0, 0.09*0.09, 0.3},
                    collisions::GatheringEvent{5, 0, 0.12*0.12, 0.4},
                    collisions::GatheringEvent{4, 0, 0.15*0.15, 0.5},
                    collisions::GatheringEvent{3, 0, 0.18*0.18, 0.6},
                  }, 
                  CompareEvents())
      );
    }

    provider.clear_gatherers();
    provider.clear_objects();
  }

  WHEN("Multiple gatherers") {
    auto& provider = collisions::LootCharacterProvider::instance();

    provider.add_object<collisions::Item>({{0, 0}, 0.1, 0});

    provider.add_gatherer({{-5, 0}, {5, 0}, 1.0, 0});
    provider.add_gatherer({{0, 1}, {0, -1}, 1.0, 1});
    provider.add_gatherer({{-10, 10}, {101, -100}, 0.5, 2});
    provider.add_gatherer({{-100, 100}, {10, -10}, 0.5, 3});

    THEN("Item will be gathered by the fastest gatherer") {
      const auto events = collisions::find_gather_events(provider);
      CHECK(events.front().gatherer_idx == 2);
    }

    provider.clear_gatherers();
    provider.clear_objects();
  }

  WHEN("Gatherers do not move") {
    auto& provider = collisions::LootCharacterProvider::instance();

    provider.add_object<collisions::Item>({{0, 0}, 10.0, 0});

    provider.add_gatherer({{-5, 0}, {-5, 0}, 1.0, 0});
    provider.add_gatherer({{0, 0}, {0, 0}, 1.0, 1});
    provider.add_gatherer({{-10, 10}, {-10, 10}, 100, 2});
    
    THEN("No events detected") {
      const auto events = collisions::find_gather_events(provider);
      CHECK(events.empty());
    }

    provider.clear_objects();
    provider.clear_gatherers();
  }
}