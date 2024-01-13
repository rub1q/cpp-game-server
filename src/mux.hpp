#pragma once 

#include "response.hpp"
#include "common_http.hpp"
#include "http_methods.hpp"
#include "handlers.hpp"

#include <vector>
#include <regex>
#include <memory>

#include <boost/algorithm/string.hpp>
#include <boost/url/url.hpp>

namespace mux {

using namespace common;
using namespace std::literals;

namespace ct = content_type;
namespace urls = boost::urls;

enum class MatchError : std::uint8_t {
  NoError,
  MethodMismatch,
  NotFound
};

struct RouteMatch {
  const http_handler::Handler* handler;
  MatchError error; 

public:
  RouteMatch() = default;

  RouteMatch(const RouteMatch&) = default;
  RouteMatch(RouteMatch&&) = default;

  RouteMatch& operator=(const RouteMatch&) = default;
  RouteMatch& operator=(RouteMatch&&) = default;
};

class Route final {
public:
  Route();

  Route* handler_func(http_handler::HandlerFunc::Type&& func) noexcept;
  Route* handler(std::unique_ptr<http_handler::Handler> handler);

  Route* not_allowed_handler(http_handler::HandlerFunc::Type&& handler);

  Route* methods(http_methods::Method allowed_methods) noexcept;

  void path(std::string_view path) noexcept;

  [[nodiscard]] const http_handler::Handler& handler() const noexcept; 
  [[nodiscard]] const http_handler::Handler& not_allowed_handler() const;

  [[nodiscard]] const http_methods::AllowedMethod& allowed_methods() const noexcept;

  [[nodiscard]] const std::string& path() const noexcept;

private:
  std::unique_ptr<http_handler::Handler> handler_;
  std::unique_ptr<http_handler::Handler> not_allowed_handler_;

  http_methods::AllowedMethod allowed_methods_;

  std::string path_;
}; 
 
class Router final {
public:
  Route* handle_func(std::string_view pattern, http_handler::HandlerFunc::Type&& handler);
  
  void set_route(std::unique_ptr<Route> route);

  template <typename Body, typename Allocator>
  [[nodiscard]] RouteMatch process(http_request_t<Body, Allocator>& req) const { 
    // decode url
    urls::pct_string_view encoded_url = req.target();

    std::string path; 
    path.resize(encoded_url.decoded_size());

    encoded_url.decode({}, urls::string_token::assign_to(path));      
    boost::trim_right(path);

    // remove last '/'
    if (path.length() > 1 && path.ends_with("/"sv) && !path.ends_with("//"sv)) {
      path = path.substr(0, path.length()-1);
    }

    req.target(path);

    RouteMatch match;

    for (const auto& route : routes_) {
      if (std::regex_match(path, std::regex(route->path()))) {
        if (!route->allowed_methods().is_allowed(req.method_string())) {
          match.error = MatchError::MethodMismatch;
          match.handler = &route->not_allowed_handler();

          break;
        }

        match.error = MatchError::NoError;
        match.handler = &route->handler();

        break;
      }

      match.error = MatchError::NotFound;
      match.handler = nullptr;
    }

    return match;
  } 

private:
  std::vector<std::unique_ptr<Route>> routes_;
};

} // namespace mux