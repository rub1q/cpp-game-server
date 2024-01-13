#include "collisions.hpp"

#include <cassert>
#include <algorithm>
#include <stdexcept>

namespace collisions {

using namespace std::literals;

bool CollectionResult::is_collected(double collect_radius) const noexcept {
  return proj_ratio >= 0 && proj_ratio <= 1 && sq_distance <= collect_radius * collect_radius;
}

CollectionResult try_collect_point(geom::Position a, geom::Position b, geom::Position c) noexcept {
  // Проверим, что перемещение ненулевое.
  // Тут приходится использовать строгое равенство, а не приближённое,
  // поскольку при сборе предметов придётся учитывать перемещение даже на небольшое расстояние
  assert(b.x != a.x || b.y != a.y);
  
  const double u_x = c.x - a.x;
  const double u_y = c.y - a.y;
  const double v_x = b.x - a.x;
  const double v_y = b.y - a.y;
  const double u_dot_v = u_x * v_x + u_y * v_y;
  const double u_len2 = u_x * u_x + u_y * u_y;
  const double v_len2 = v_x * v_x + v_y * v_y;
  const double proj_ratio = u_dot_v / v_len2;
  const double sq_distance = u_len2 - (u_dot_v * u_dot_v) / v_len2;

  return CollectionResult {
    .sq_distance = sq_distance, 
    .proj_ratio = proj_ratio
  };
}

std::size_t LootCharacterProvider::objects_count() const noexcept {
  return objects_.size();
}

const Object& LootCharacterProvider::get_object(const std::size_t idx) const {
  if (idx >= objects_count()) {
    throw std::out_of_range("Invalid items index"s);
  }
  
  return *objects_[idx].get();
}

std::size_t LootCharacterProvider::gatherers_count() const noexcept {
  return gatherers_.size();
}

const Gatherer& LootCharacterProvider::get_gatherer(const std::size_t idx) const {
  if (idx >= gatherers_count()) {
    throw std::out_of_range("Invalid gatherer index"s);
  }  
  
  return gatherers_[idx]; 
} 

LootCharacterProvider& LootCharacterProvider::instance() noexcept {
  static LootCharacterProvider lcp;
  return lcp;
}

void LootCharacterProvider::add_gatherer(Gatherer gatherer) {
  gatherers_.push_back(std::move(gatherer));
} 

void LootCharacterProvider::clear_objects() noexcept {
  objects_.clear();
}

void LootCharacterProvider::clear_gatherers() noexcept {
  gatherers_.clear();
}

GatheringEvents find_gather_events(const ItemGathererProvider& provider) {
  GatheringEvents detected_events;

  for (std::size_t g = 0; g < provider.gatherers_count(); g++) {
    decltype(auto) gatherer = provider.get_gatherer(g);

    if ((gatherer.start_pos.x == gatherer.end_pos.x) && 
        (gatherer.start_pos.y == gatherer.end_pos.y)) {
      continue;
    }

    for (std::size_t i = 0; i < provider.objects_count(); i++) {
      decltype(auto) obj = provider.get_object(i);

      const auto collect_result = try_collect_point(gatherer.start_pos, gatherer.end_pos, obj.position);

      if (collect_result.is_collected(gatherer.width + obj.width)) {
        GatheringEvent evt {
          .object_idx = i,
          .gatherer_idx = g,
          .sq_distance = collect_result.sq_distance,
          .time = collect_result.proj_ratio
        };

        detected_events.push_back(std::move(evt));
      }
    }
  }

  if (!detected_events.empty()) {
    std::sort(detected_events.begin(), detected_events.end(), 
              [](const GatheringEvent& e_l, const GatheringEvent& e_r) {
                return e_l.time < e_r.time;
              });
  }

  return detected_events; 
}

} // namespace collisions 