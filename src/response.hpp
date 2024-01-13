#pragma once 

#include "game.hpp"
#include "common_http.hpp"
#include "content_type.hpp"
#include "config.hpp"

#include <string>
#include <filesystem>
#include <boost/json.hpp>

namespace response {

namespace json = boost::json;
namespace ct = content_type;

using namespace common;
using namespace std::literals;

namespace basic_json_body {

inline std::string bad_response(std::string_view code, std::string_view message) {
  const auto ans = json::serialize(json::object{
    {"code"sv, std::move(code)}, 
    {"message"sv, std::move(message)}
  });

  return ans;  
}

inline std::string invalid_argument(std::string_view message = "Invalid Argument"sv) {
  const auto ans = bad_response("invalidArgument"sv, std::move(message)); 
  return ans;
}

inline std::string invalid_method(std::string_view message = "Invalid Method"sv) {
  const auto ans = bad_response("invalidMethod"sv, std::move(message)); 
  return ans;  
}

inline std::string bad_request(std::string_view message = "Bad Request"sv) {
  const auto ans = bad_response("badRequest"sv, std::move(message)); 
  return ans;  
}

inline std::string map_not_found(std::string_view message = "Map Not Found"sv) {
  const auto ans = bad_response("mapNotFound"sv, std::move(message));
  return ans;
}

inline std::string header_missing(std::string_view message) {
  const auto ans = bad_response("invalidToken"sv, std::move(message));
  return ans;
}

} // namespace basic_json_body 

template <typename BodyType = http::string_body::value_type>
class ResponseFields {
public:
  using HttpFields = std::unordered_map<http::field, std::string>;
  
  ResponseFields() = default;

  ResponseFields(ResponseFields&&) = default;
  ResponseFields(const ResponseFields&) = default;

  ResponseFields& operator=(ResponseFields&&) = default;
  ResponseFields& operator=(const ResponseFields&) = default;

  explicit ResponseFields(const unsigned ver, bool keep_alive)
    : version_(ver)
    , keep_alive_(keep_alive) {
  }

  ResponseFields& add_field(http::field field, std::string_view value) {
    http_fields_.try_emplace(field, std::string(value));
    return *this;
  }

  virtual ResponseFields& add_body(BodyType&& body) {
    body_ = std::forward<decltype(body)>(body);
    return *this;
  }

  const HttpFields& http_fields() const noexcept {
    return http_fields_;
  }

  const BodyType& body() const noexcept {
    return body_;
  }

  BodyType&& body() {
    return std::move(body_);
  }

  bool keep_alive() const noexcept {
    return keep_alive_;
  }

  http::status http_status() const noexcept {
    return status_;
  }

  unsigned http_version() const noexcept {
    return version_;
  }

  virtual ~ResponseFields() = default;

protected:
  bool keep_alive_;
  
  http::status status_;
  unsigned version_;

  BodyType body_;
  HttpFields http_fields_;
};

// Bad responses

struct BadRequestBase : public ResponseFields<> {
  BadRequestBase() = delete;
  explicit BadRequestBase(const unsigned ver, bool keep_alive, ct::Type content_type);
};

template <ct::Type T>
struct BadRequest final : public BadRequestBase {
  BadRequest() = delete;  
};

template <>
struct BadRequest<ct::text_plain> final : public BadRequestBase {
  explicit BadRequest(const unsigned ver, bool keep_alive, std::string_view msg = "Bad Request"sv)
    : BadRequestBase(ver, keep_alive, ct::text_plain) {
    
    body_ = std::move(msg);
  }
};

template <>
struct BadRequest<ct::app_json> final : public BadRequestBase {
  explicit BadRequest(const unsigned ver, bool keep_alive)
    : BadRequestBase(ver, keep_alive, ct::app_json) { 
  }
};

struct InternalServerErrorBase : public ResponseFields<> {
  InternalServerErrorBase() = delete;
  explicit InternalServerErrorBase(const unsigned ver, bool keep_alive, ct::Type content_type);
};

template <ct::Type T>
struct InternalServerError final : public InternalServerErrorBase {
  InternalServerError() = delete;
};

template <>
struct InternalServerError<ct::text_plain> final : public InternalServerErrorBase {
  explicit InternalServerError(const unsigned ver, bool keep_alive, std::string_view msg = "Internal Server Error"sv)
    : InternalServerErrorBase(ver, keep_alive, ct::text_plain) {
    
    body_ = std::move(msg);
  }
};

template <>
struct InternalServerError<ct::app_json> final : public InternalServerErrorBase {
  explicit InternalServerError(const unsigned ver, bool keep_alive)
    : InternalServerErrorBase(ver, keep_alive, ct::app_json) { 
  } 
};

struct MethodNotAllowedBase : public ResponseFields<> {
  MethodNotAllowedBase() = delete;
  explicit MethodNotAllowedBase(const unsigned ver, bool keep_alive, ct::Type content_type, std::string_view allowed);
};

template <ct::Type T>
struct MethodNotAllowed final : public MethodNotAllowedBase {
  MethodNotAllowed() = delete;
};

template <>
struct MethodNotAllowed<ct::text_plain> final : public MethodNotAllowedBase {
  explicit MethodNotAllowed(const unsigned ver, bool keep_alive, std::string_view allowed_methods, std::string_view msg = "Invalid method"sv) 
    : MethodNotAllowedBase(ver, keep_alive, ct::text_plain, std::move(allowed_methods)) {

    body_ = std::move(msg);
  } 
};

template <>
struct MethodNotAllowed<ct::app_json> final : public MethodNotAllowedBase {
  explicit MethodNotAllowed(const unsigned ver, bool keep_alive, std::string_view allowed_methods)
    : MethodNotAllowedBase(ver, keep_alive, ct::app_json, std::move(allowed_methods)) {
  } 
};

struct NotFoundBase : public ResponseFields<> {
  NotFoundBase() = delete;
  explicit NotFoundBase(const unsigned ver, bool keep_alive, ct::Type content_type);
};

template <ct::Type T>
struct NotFound final : public NotFoundBase {
  NotFound() = delete;
};

template <>
struct NotFound<ct::text_plain> final : public NotFoundBase {
  explicit NotFound(const unsigned ver, bool keep_alive, std::string_view msg = "Not Found"sv) 
    : NotFoundBase(ver, keep_alive, ct::text_plain) {

    body_ = std::move(msg);
  }
};

template <>
struct NotFound<ct::app_json> final : public NotFoundBase {
  explicit NotFound(const unsigned ver, bool keep_alive)
    : NotFoundBase(ver, keep_alive, ct::app_json) {
  }
};

struct UnauthorizedBase : public ResponseFields<> {
  UnauthorizedBase() = delete;
  explicit UnauthorizedBase(const unsigned ver, bool keep_alive, ct::Type content_type);
};

template <ct::Type T>
struct Unauthorized final : public UnauthorizedBase {
  Unauthorized() = delete;
};

template <>
struct Unauthorized<ct::text_plain> final : public UnauthorizedBase {
  explicit Unauthorized(const unsigned ver, bool keep_alive, std::string_view msg = "Unauthorized"sv) 
    : UnauthorizedBase(ver, keep_alive, ct::text_plain) {

    body_ = std::move(msg);
  } 
}; 

template <>
struct Unauthorized<ct::app_json> final : public UnauthorizedBase {
  explicit Unauthorized(const unsigned ver, bool keep_alive)
    : UnauthorizedBase(ver, keep_alive, ct::app_json) {
  }
};

// Good responses

struct MapInfo final : public ResponseFields<> {
public:  
  explicit MapInfo(const unsigned ver, bool keep_alive, const model::Game& game, std::string_view id);

private:
  [[nodiscard]] std::string get_map_info(const model::Game& game, std::string_view id) const;  
};

struct MapsShortList final : public ResponseFields<> {
  explicit MapsShortList(const unsigned ver, bool keep_alive, const model::Game& game); 

private:
  [[nodiscard]] std::string get_map_list(const model::Game& game) const; 
};

struct File final : public ResponseFields<http::file_body::value_type> {
  explicit File(const unsigned ver, bool keep_alive, std::filesystem::path file_path);
};

struct SuccessJoin final : public ResponseFields<> {
  explicit SuccessJoin(const unsigned ver, bool keep_alive, std::string_view token, model::Character::Id id);
}; 

struct PlayersList final : public ResponseFields<> {
  explicit PlayersList(const unsigned ver, bool keep_alive, const model::GameSession::Characters& characters);
};

struct GameState final : public ResponseFields<> {
  explicit GameState(const unsigned ver, bool keep_alive, const model::GameSession& game_session);
};

struct MovePlayer final : public ResponseFields<> {
  explicit MovePlayer(const unsigned ver, bool keep_alive);
};

struct UpdatePlayersPositions final : public ResponseFields<> {
  explicit UpdatePlayersPositions(const unsigned ver, bool keep_alive); 
};
 
template <typename ResponseType, typename BodyType>
inline ResponseType make_basic_response(ResponseFields<BodyType>&& fields) {
  ResponseType response {fields.http_status(), fields.http_version()};

  response.body() = std::move(fields.body());
  response.keep_alive(fields.keep_alive());
  
  for (const auto& [field, value] : fields.http_fields()) {
    response.set(field, value);
  }

  return response; 
}

inline http_string_response_t make(ResponseFields<http::string_body::value_type> response_fields) {
  auto response = make_basic_response<http_string_response_t, 
    http::string_body::value_type>(std::move(response_fields));

  response.content_length(response.body().length());

  return response;
}

inline http_file_response_t make(ResponseFields<http::file_body::value_type> response_fields) {
  auto response = make_basic_response<http_file_response_t, 
    http::file_body::value_type>(std::move(response_fields));

  response.prepare_payload();

  return response;
}

} // namespace response  