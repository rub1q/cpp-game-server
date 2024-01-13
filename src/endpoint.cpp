#include "endpoint.hpp"
#include "config.hpp"
#include "response.hpp"

#include <boost/json.hpp>
#include <stdexcept>

namespace endpoint {

using namespace std::literals;

namespace json = boost::json;
namespace ct = content_type;

app::Token::Type extract_token(std::string_view value) {
  const app::Token::Type token(value);
  return token.substr("Bearer"s.length() + 1, value.length());
}

template <typename Fn>
http_handler::HandlerFunc::Type auth_middleware(Fn&& next) {
  return [next](http_string_request_t&& req) -> http_response_t {
    const auto it = req.base().find("Authorization"sv);

    if ((it == req.base().cend()) || !it->value().starts_with("Bearer"sv)) {
      return response::make(response::Unauthorized<ct::app_json>(req.version(), req.keep_alive())
        .add_field(http::field::cache_control, "no-cache"sv)
        .add_body(response::basic_json_body::header_missing("Authorization header is missing"sv))
      );
    } 

    const auto player = app::Players::instance().find_by_token(extract_token(it->value()));
    if (!player) {
      return response::make(response::Unauthorized<ct::app_json>(req.version(), req.keep_alive())
        .add_field(http::field::cache_control, "no-cache"sv)
        .add_body(response::basic_json_body::bad_response("unknownToken"sv, "Player token has not been found"sv))
      );
    }

    return next(player, std::move(req));
  };
}

std::unique_ptr<mux::Route> GetIndex::route() {
  auto route = std::make_unique<mux::Route>();

  route->path("^/$"sv);
  route->methods(http_methods::Method::get | http_methods::Method::head);
  route->handler_func(http_handler::file_server(config::get().server.www_root));

  return route;
}

std::unique_ptr<mux::Route> GetFile::route() {
  auto route = std::make_unique<mux::Route>();

  route->path("^(?!(^/api/))(/.*)$"sv);
  route->methods(http_methods::Method::get | http_methods::Method::head);
  route->handler_func(http_handler::file_server(config::get().server.www_root));

  return route;
}

std::unique_ptr<mux::Route> GetMapsList::route() {
  auto route = std::make_unique<mux::Route>();

  route->path("^/api/v1/maps$"sv);
  route->methods(http_methods::Method::get | http_methods::Method::head);
  route->handler_func([](http_string_request_t&& req) {
    return response::make(response::MapsShortList(req.version(), req.keep_alive(), *config::get().game));
  });

  return route;
}

http_response_t GameJoin::handler(http_string_request_t&& req) {
  const auto it = req.base().find("Content-Type"sv);

  if ((it == req.base().cend()) || it->value() != ct::get_as_text(ct::app_json)) {
    return response::make(response::BadRequest<ct::app_json>(req.version(), req.keep_alive())
      .add_field(http::field::cache_control, "no-cache"sv)
      .add_body(response::basic_json_body::invalid_argument("Invalid Content-Type header"sv))
    );
  }
    
  //1. Найти игровой сеанс, соответствующий карте, на которой хочет играть клиент.
  //2. Внутри игрового сеанса добавить нового пса с указанным именем и сгенерированным id.

  std::string username, req_map_id;

  try {
    json::object req_body = json::parse(req.body()).as_object();
    username = req_body.at("userName"sv).as_string();

    if (username.empty()) {
      return response::make(response::BadRequest<ct::app_json>(req.version(), req.keep_alive())
        .add_field(http::field::cache_control, "no-cache"sv)
        .add_body(response::basic_json_body::invalid_argument("Invalid name"sv))
      );
    }

    req_map_id = req_body.at("mapId"sv).as_string();

  } catch (...) {
    return response::make(response::BadRequest<ct::app_json>(req.version(), req.keep_alive())
      .add_field(http::field::cache_control, "no-cache"sv)
      .add_body(response::basic_json_body::invalid_argument("Join game request parse error"sv))
    );
  }

  const auto map = config::get().game->find_map(model::Map::Id(req_map_id));

  if (!map) {
    return response::make(response::NotFound<ct::app_json>(req.version(), req.keep_alive())
      .add_field(http::field::cache_control, "no-cache"sv)
      .add_body(response::basic_json_body::map_not_found())
    ); 
  }

  auto session = config::get().game->get_session(map);
  auto dog = model::create_character<model::Dog>(std::move(username), session->config().bag_capacity);

  auto [id, character] = session->add_character(std::move(dog));
  const auto& [token, player] = app::Players::instance().new_player(character, *session);

  return response::make(response::SuccessJoin(req.version(), req.keep_alive(), std::move(token), id));   
}

std::unique_ptr<mux::Route> GameJoin::route() {
  auto route = std::make_unique<mux::Route>();

  route->path("^/api/v1/game/join$"sv);
  route->methods(http_methods::Method::post);
  route->handler_func(std::bind(&GameJoin::handler, this, std::placeholders::_1));

  route->not_allowed_handler([allowed = route->allowed_methods()](http_string_request_t&& req) {
    return response::make(response::MethodNotAllowed<ct::app_json>(req.version(), req.keep_alive(), allowed.as_string())
      .add_field(http::field::cache_control, "no-cache"sv)
      .add_body(response::basic_json_body::invalid_method("Expected: "s + allowed.as_string()))
    );
  });

  return route;
}

http_response_t GetPlayers::handler(app::Player *const player, http_string_request_t&& req) { 
  return response::make(response::PlayersList(req.version(), req.keep_alive(), player->game_session().characters()));    
}

std::unique_ptr<mux::Route> GetPlayers::route() {
  auto route = std::make_unique<mux::Route>();

  route->path("^/api/v1/game/players$"sv);
  route->methods(http_methods::Method::get | http_methods::Method::head);
  route->handler_func(auth_middleware(std::bind(&GetPlayers::handler, this, std::placeholders::_1, std::placeholders::_2)));

  route->not_allowed_handler([allowed = route->allowed_methods()](http_string_request_t&& req) {
    return response::make(response::MethodNotAllowed<ct::app_json>(req.version(), req.keep_alive(), allowed.as_string())
      .add_field(http::field::cache_control, "no-cache"sv)
      .add_body(response::basic_json_body::invalid_method("Expected: "s + allowed.as_string()))
    );
  });

  return route;
}

http_response_t GetMapInfo::handler(http_string_request_t&& req) {
  const auto path = req.target(); 
  const auto id = path.substr(path.find_last_of("/"sv) + 1, path.size()); 

  if (const auto map = config::get().game->find_map(model::Map::Id(id.data()))) {
    return response::make(response::MapInfo(req.version(), req.keep_alive(), *config::get().game, *map->get_id())); 
  }

  return response::make(response::NotFound<ct::app_json>(req.version(), req.keep_alive())
    .add_body(response::basic_json_body::map_not_found())
  );
}

std::unique_ptr<mux::Route> GetMapInfo::route() {
  auto route = std::make_unique<mux::Route>();

  route->path("^/api/v1/maps/\\w+$"sv);
  route->methods(http_methods::Method::get | http_methods::Method::head);
  route->handler_func(std::bind(&GetMapInfo::handler, this, std::placeholders::_1));

  return route;
}

http_response_t GetGameState::handler(app::Player *const player, http_string_request_t&& req) {
  return response::make(response::GameState(req.version(), req.keep_alive(), player->game_session()));
}

std::unique_ptr<mux::Route> GetGameState::route() {
  auto route = std::make_unique<mux::Route>();

  route->path("^/api/v1/game/state$"sv);
  route->methods(http_methods::Method::get | http_methods::Method::head);
  route->handler_func(auth_middleware(std::bind(&GetGameState::handler, this, std::placeholders::_1, std::placeholders::_2)));

  route->not_allowed_handler([allowed = route->allowed_methods()](http_string_request_t&& req) {
    return response::make(response::MethodNotAllowed<ct::app_json>(req.version(), req.keep_alive(), allowed.as_string())
      .add_field(http::field::cache_control, "no-cache"sv)
      .add_body(response::basic_json_body::invalid_method("Expected: "s + allowed.as_string()))
    );
  });  

  return route;
}

http_response_t PlayerAction::handler(http_string_request_t&& req) {
  const auto it = req.base().find("Content-Type"sv);

  if ((it == req.base().cend()) || it->value() != ct::get_as_text(ct::app_json)) {
    return response::make(response::BadRequest<ct::app_json>(req.version(), req.keep_alive())
      .add_field(http::field::cache_control, "no-cache"sv)
      .add_body(response::basic_json_body::invalid_argument("Invalid Content-Type header"sv))
    );
  }

  std::string direction;

  try {
    json::object req_body = json::parse(req.body()).as_object();
    direction = req_body.at("move"sv).as_string();

    if (!direction.empty()) {
      const auto pos = direction.find_first_of("LRUD"sv);

      if (direction.length() > 1 || (pos == std::string::npos)) {
        throw std::invalid_argument("Invalid move direction value"s);
      }
    }

  } catch (...) {
    return response::make(response::BadRequest<ct::app_json>(req.version(), req.keep_alive())
      .add_field(http::field::cache_control, "no-cache"sv)
      .add_body(response::basic_json_body::invalid_argument("Failed to parse action"sv))
    );
  }
  
  return auth_middleware([&direction](app::Player *const player, http_string_request_t&& req) {
    auto& character = player->character();
    character.move(model::Character::Direction(direction), player->game_session().config().characters_speed);

    return response::make(response::MovePlayer(req.version(), req.keep_alive()));
  })(std::move(req));
}

std::unique_ptr<mux::Route> PlayerAction::route() {
  auto route = std::make_unique<mux::Route>();

  route->path("^/api/v1/game/player/action$"sv);
  route->methods(http_methods::Method::post);
  route->handler_func(std::bind(&PlayerAction::handler, this, std::placeholders::_1));

  route->not_allowed_handler([allowed = route->allowed_methods()](http_string_request_t&& req) {
    return response::make(response::MethodNotAllowed<ct::app_json>(req.version(), req.keep_alive(), allowed.as_string())
      .add_field(http::field::cache_control, "no-cache"sv)
      .add_body(response::basic_json_body::invalid_method("Expected: "s + allowed.as_string()))
    );
  });

  return route;  
}

http_response_t Tick::handler(http_string_request_t&& req) {
  const auto it = req.base().find("Content-Type"sv);

  if ((it == req.base().cend()) || it->value() != ct::get_as_text(ct::app_json)) {
    return response::make(response::BadRequest<ct::app_json>(req.version(), req.keep_alive())
      .add_field(http::field::cache_control, "no-cache"sv)
      .add_body(response::basic_json_body::invalid_argument("Invalid Content-Type header"sv))
    );
  }  

  std::int64_t deltaTime;

  try {
    json::object req_body = json::parse(req.body()).as_object();
    deltaTime = req_body.at("timeDelta"sv).as_int64();
  } catch (...) {
    return response::make(response::BadRequest<ct::app_json>(req.version(), req.keep_alive())
      .add_field(http::field::cache_control, "no-cache"sv)
      .add_body(response::basic_json_body::invalid_argument("Failed to parse tick request JSON"sv))
    );
  }  

  config::get().game->refresh_state(deltaTime);

  return response::make(response::UpdatePlayersPositions(req.version(), req.keep_alive()));
}

std::unique_ptr<mux::Route> Tick::route() {
  auto route = std::make_unique<mux::Route>();

  route->path("^/api/v1/game/tick$"sv);
  route->methods(http_methods::Method::post);
  route->handler_func(std::bind(&Tick::handler, this, std::placeholders::_1));

  return route;
}

} // namespace endpoint