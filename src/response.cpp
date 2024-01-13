#include "response.hpp"
#include "extra_data.hpp"

namespace response {

BadRequestBase::BadRequestBase(const unsigned ver, bool keep_alive, ct::Type content_type)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::bad_request;
  http_fields_[http::field::content_type] = std::move(ct::get_as_text(content_type));
} 

InternalServerErrorBase::InternalServerErrorBase(const unsigned ver, bool keep_alive, ct::Type content_type)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::internal_server_error;
  http_fields_[http::field::content_type] = std::move(ct::get_as_text(content_type));
}

MethodNotAllowedBase::MethodNotAllowedBase(const unsigned ver, bool keep_alive, ct::Type content_type, std::string_view allowed)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::method_not_allowed;

  http_fields_[http::field::allow] = std::move(allowed);
  http_fields_[http::field::content_type] = std::move(ct::get_as_text(content_type));
}

NotFoundBase::NotFoundBase(const unsigned ver, bool keep_alive, ct::Type content_type)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::not_found;
  http_fields_[http::field::content_type] = std::move(ct::get_as_text(content_type));
}

UnauthorizedBase::UnauthorizedBase(const unsigned ver, bool keep_alive, ct::Type content_type)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::unauthorized;
  http_fields_[http::field::content_type] = std::move(ct::get_as_text(content_type));
}

MapInfo::MapInfo(const unsigned ver, bool keep_alive, const model::Game& game, std::string_view id)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::ok;
  http_fields_[http::field::content_type] = std::move(ct::get_as_text(ct::app_json));

  body_ = std::move(get_map_info(game, id));
}

std::string MapInfo::get_map_info(const model::Game& game, std::string_view id) const {
  json::object obj;

  const auto map = game.find_map(model::Map::Id(id.data()));

  obj["id"] = *map->get_id();
  obj["name"] = map->get_name();

  json::array roads_array; 
  roads_array.reserve(map->get_roads().size());
  
  for (const auto& road : map->get_roads()) {
    json::object road_obj;

    road_obj["x0"] = road.get_start().x;
    road_obj["y0"] = road.get_start().y;

    if (road.is_horizontal())
      road_obj["x1"] = road.get_end().x;
    else 
      road_obj["y1"] = road.get_end().y;

    roads_array.push_back(std::move(road_obj));
  }

  obj["roads"] = std::move(roads_array);

  json::array build_array;
  build_array.reserve(map->get_buildings().size());

  for (const auto& building : map->get_buildings()) {
    json::object build_obj;

    build_obj["x"] = building.get_bounds().position.x;
    build_obj["y"] = building.get_bounds().position.y;

    build_obj["w"] = building.get_bounds().size.width;
    build_obj["h"] = building.get_bounds().size.height;

    build_array.push_back(std::move(build_obj));
  }

  obj["buildings"] = std::move(build_array); 

  json::array offices_array;
  offices_array.reserve(map->get_offices().size());

  for (const auto& office : map->get_offices()) {
    json::object office_obj;

    office_obj["id"] = *office.get_id();

    office_obj["x"] = office.get_position().x;
    office_obj["y"] = office.get_position().y;

    office_obj["offsetX"] = office.get_offset().dx;
    office_obj["offsetY"] = office.get_offset().dy;

    offices_array.push_back(std::move(office_obj));
  }

  obj["offices"] = std::move(offices_array); 
  obj["lootTypes"] = extra_data::LootTypes::instance().get(map->get_id());

  return json::serialize(obj);  
}

MapsShortList::MapsShortList(const unsigned ver, bool keep_alive, const model::Game& game)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::ok;
  http_fields_[http::field::content_type] = std::move(ct::get_as_text(ct::app_json));

  body_ = std::move(get_map_list(game));
}  

std::string MapsShortList::get_map_list(const model::Game& game) const {
  json::array arr;
  arr.reserve(game.get_maps().size());

  for (const auto& map : game.get_maps()) {
    json::object obj;

    obj["id"] = *map.get_id();
    obj["name"] = map.get_name();
    
    arr.push_back(std::move(obj));
  }

  return json::serialize(arr);  
}

File::File(const unsigned ver, bool keep_alive, std::filesystem::path file_path)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::ok;

  std::string ext(file_path.extension());
  http_fields_[http::field::content_type] = std::move(ct::get_as_text(ct::get_ext_as_type(std::move(ext))));

  http::file_body::value_type value;
  boost::system::error_code ec;

  if (value.open(file_path.c_str(), beast::file_mode::read, ec); ec) { 
    throw std::runtime_error("Failed to open the file"s);
  }

  body_ = std::move(value);
  value.close();
} 

SuccessJoin::SuccessJoin(const unsigned ver, bool keep_alive, std::string_view token, model::Character::Id id)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::ok;

  http_fields_[http::field::content_type] = std::move(ct::get_as_text(ct::app_json));
  http_fields_[http::field::cache_control] = "no-cache"sv;

  body_ = json::serialize(json::object{
    { "authToken"sv, std::move(token) },
    { "playerId"sv, id }
  });
}

PlayersList::PlayersList(const unsigned ver, bool keep_alive, const model::GameSession::Characters& characters)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::ok;

  http_fields_[http::field::content_type] = std::move(ct::get_as_text(ct::app_json));
  http_fields_[http::field::cache_control] = "no-cache"sv;

  json::object obj;
  
  for (const auto& [id, ch] : characters) {
    obj[std::to_string(id)] = json::object{{"name"sv, ch->name()}};
  }

  body_ = json::serialize(std::move(obj)); 
}

GameState::GameState(const unsigned ver, bool keep_alive, const model::GameSession& game_session)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::ok;

  http_fields_[http::field::content_type] = std::move(ct::get_as_text(ct::app_json));
  http_fields_[http::field::cache_control] = "no-cache"sv;

  json::object players;

  for (const auto& [id, ch] : game_session.characters()) {
    json::array pos, speed, bag;

    pos.push_back(ch->position().x);
    pos.push_back(ch->position().y);

    speed.push_back(ch->speed().x);
    speed.push_back(ch->speed().y);

    json::object player_info;

    player_info["pos"] = std::move(pos);
    player_info["speed"] = std::move(speed);
    player_info["dir"] = ch->direction().as_letter();  

    bag.reserve(ch->bagpack.get().size());

    for (const auto& loot : ch->bagpack.get()) {
      bag.emplace_back(json::object {
        {"id", loot.first}, 
        {"type", loot.second->type()}
      });
    }

    player_info["bag"] = std::move(bag);
    player_info["score"] = ch->score();

    players[std::to_string(id)] = std::move(player_info);
  }

  json::object lost_objects; 

  for (const auto& [id, loot] : game_session.lost_objects()) {
    json::array pos;

    pos.push_back(loot->position().x);
    pos.push_back(loot->position().y);

    json::object loot_info;

    loot_info["type"] = loot->type();    
    loot_info["pos"] = std::move(pos);
    
    lost_objects[std::to_string(id)] = std::move(loot_info);
  }

  body_ = json::serialize(json::object {
    {"players"sv, std::move(players)},
    {"lostObjects"sv, std::move(lost_objects)}
  });
} 

MovePlayer::MovePlayer(const unsigned ver, bool keep_alive)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::ok;

  http_fields_[http::field::content_type] = std::move(ct::get_as_text(ct::app_json));
  http_fields_[http::field::cache_control] = "no-cache"sv;

  body_ = json::serialize(json::object{});
}

UpdatePlayersPositions::UpdatePlayersPositions(const unsigned ver, bool keep_alive)
  : ResponseFields(ver, keep_alive) {
  
  status_ = http::status::ok;

  http_fields_[http::field::content_type] = std::move(ct::get_as_text(ct::app_json));
  http_fields_[http::field::cache_control] = "no-cache"sv;

  body_ = json::serialize(json::object{});
} 

} // namespace response