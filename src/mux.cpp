#include "mux.hpp"

namespace mux {

Route::Route() {
  not_allowed_handler_ = std::make_unique<http_handler::HandlerFunc>([this](http_string_request_t&& req) {
    return response::make(response::MethodNotAllowed<ct::text_plain>(req.version(), req.keep_alive(), allowed_methods().as_string()));  
  });
}

Route* Route::handler(std::unique_ptr<http_handler::Handler> handler) {
  handler_ = std::move(handler);
  return this;
}

Route* Route::handler_func(http_handler::HandlerFunc::Type&& func) noexcept {
  return handler(std::make_unique<http_handler::HandlerFunc>(std::move(func)));
} 

Route* Route::methods(http_methods::Method allowed_methods) noexcept {
  allowed_methods_.set(allowed_methods);
  return this;
}

[[nodiscard]] const http_handler::Handler& Route::handler() const noexcept {
  return *handler_.get();
}

[[nodiscard]] const http_handler::Handler& Route::not_allowed_handler() const { 
  return *not_allowed_handler_.get();
}

[[nodiscard]] const http_methods::AllowedMethod& Route::allowed_methods() const noexcept {
  return allowed_methods_;
} 

Route* Route::not_allowed_handler(http_handler::HandlerFunc::Type&& handler) {
  not_allowed_handler_.reset(new http_handler::HandlerFunc(std::move(handler)));
  return this;
}

void Route::path(std::string_view path) noexcept {
  path_ = std::string(path);
}

[[nodiscard]] const std::string& Route::path() const noexcept {
  return path_;
}

Route* Router::handle_func(std::string_view pattern, http_handler::HandlerFunc::Type&& handler) { 
  auto& el = routes_.emplace_back(std::make_unique<Route>());
  el->handler_func(std::move(handler));

  return el.get();
}

void Router::set_route(std::unique_ptr<Route> route) {
  routes_.emplace_back(std::move(route));
}

} // namespace mux