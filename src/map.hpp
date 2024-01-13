#pragma once

#include "tagged.hpp"
#include "geometry.hpp"

#include <vector>
#include <unordered_map>

namespace model {

class Road {
  struct HorizontalTag {
    explicit HorizontalTag() = default;
  };

  struct VerticalTag {
    explicit VerticalTag() = default;
  };

public:
  constexpr static HorizontalTag HORIZONTAL{};
  constexpr static VerticalTag VERTICAL{};

  constexpr static geom::Coord WIDTH { 0.8 };

  Road() = default;
  
  Road(HorizontalTag, geom::Position start, geom::Coord end_x) noexcept
    : start_{start}
    , end_{end_x, start.y} {
  }

  Road(VerticalTag, geom::Position start, geom::Coord end_y) noexcept
    : start_{start}
    , end_{start.x, end_y} {
  }

  bool is_horizontal() const noexcept;
  bool is_vertical() const noexcept;

  geom::Position get_start() const noexcept;
  geom::Position get_end() const noexcept;

private:
  geom::Position start_;
  geom::Position end_;
};

class Building {
public:
  explicit Building(geom::Rectangle bounds) noexcept
    : bounds_{bounds} {
  }

  const geom::Rectangle& get_bounds() const noexcept;

private:
  geom::Rectangle bounds_;
};

class Office {
public:
  using Id = util::Tagged<std::string, Office>;

  constexpr static geom::Coord WIDTH { 0.5 };

  Office(Id id, geom::Position position, geom::Offset offset) noexcept
    : id_{std::move(id)}
    , position_{position}
    , offset_{offset} {
  }

  const Id& get_id() const noexcept;

  geom::Position get_position() const noexcept;
  geom::Offset get_offset() const noexcept;

private:
  Id id_;
  geom::Position position_;
  geom::Offset offset_;
};

class Map {
public:
  using Id        = util::Tagged<std::string, Map>;
  using Roads     = std::vector<Road>;
  using Buildings = std::vector<Building>;
  using Offices   = std::vector<Office>;

  using IdHasher = util::TaggedHasher<Map::Id>;

  Map(Id id, std::string name) noexcept
    : id_(std::move(id))
    , name_(std::move(name)) {
  }

  const Id& get_id() const noexcept;

  const std::string& get_name() const noexcept;

  const Buildings& get_buildings() const noexcept;
  const Roads& get_roads() const noexcept;
  const Offices& get_offices() const noexcept;

  const Road* get_road_by_position(const geom::Position& position) const;

  void add_road(const Road& road);
  void add_building(const Building& building);
  void add_office(const Office& office);

private:
  void add_road_position(const Road& road);

private:
  using OfficeIdToIndex = std::unordered_map<Office::Id, size_t, util::TaggedHasher<Office::Id>>;

  Id id_;
  std::string name_;

  Roads roads_;
  Buildings buildings_;

  OfficeIdToIndex warehouse_id_to_index_;
  Offices offices_;

  std::unordered_map<geom::Position, Road, geom::PositionHasher> roads_positions;
};

}  // namespace model