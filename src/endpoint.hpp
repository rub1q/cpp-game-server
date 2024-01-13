#pragma once

#include "mux.hpp"
#include "common_http.hpp"
#include "handlers.hpp"
#include "player.hpp"

namespace endpoint {

using namespace common;

struct Endpoint {
  virtual std::unique_ptr<mux::Route> route() = 0;

protected:
  Endpoint() = default;
};

struct GetIndex : public Endpoint {
  std::unique_ptr<mux::Route> route() override;
};

struct GetFile : public Endpoint {
  std::unique_ptr<mux::Route> route() override;
};

struct GetMapsList : public Endpoint {
  std::unique_ptr<mux::Route> route() override;
};

struct GameJoin : public Endpoint {
  std::unique_ptr<mux::Route> route() override; 

private:
  http_response_t handler(http_string_request_t&& req);
};

struct GetPlayers : public Endpoint {
  std::unique_ptr<mux::Route> route() override; 

private:
  http_response_t handler(app::Player *const player, http_string_request_t&& req);
};

struct GetMapInfo : public Endpoint {
  std::unique_ptr<mux::Route> route() override; 

private:
  http_response_t handler(http_string_request_t&& req);
};

struct GetGameState : public Endpoint {
  std::unique_ptr<mux::Route> route() override; 

private:
  http_response_t handler(app::Player *const player, http_string_request_t&& req);
};

struct PlayerAction : public Endpoint {
  std::unique_ptr<mux::Route> route() override; 

private: 
  http_response_t handler(http_string_request_t&& req);
};

struct Tick : public Endpoint {
  std::unique_ptr<mux::Route> route() override;  

private:
  http_response_t handler(http_string_request_t&& req); 
};

} // namespace endpoint