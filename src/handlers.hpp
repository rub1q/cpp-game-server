#pragma once

#include "common_http.hpp"
#include "content_type.hpp"
#include "response.hpp"

#include <filesystem>

namespace http_handler {

using namespace common;

namespace fs = std::filesystem;

struct Handler {
  [[nodiscard]] virtual http_response_t operator()(http_string_request_t&& req) const = 0;
};

struct HandlerFunc : public Handler {
  using Type = std::function<http_response_t(http_string_request_t&&)>; 

  [[nodiscard]] http_response_t operator()(http_string_request_t&& req) const override {
    return handler_(std::move(req));
  }

  HandlerFunc() = default;
  HandlerFunc(Type&& handler) 
    : handler_(std::move(handler)) {
  }

protected:
  Type handler_;
};

[[nodiscard]] inline bool is_sub_path(fs::path path, fs::path base) {
  path = fs::weakly_canonical(path);
  base = fs::weakly_canonical(base);

  for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
    if (p == path.end() || *p != *b) {
      return false;
    }
  }
  
  return true;  
}

inline auto file_server(fs::path root) {
  return [](http_string_request_t&& req) -> http_response_t {
    using namespace std::literals;
    namespace ct = content_type;
    
    std::stringstream path;   
    path << config::get().server.www_root.c_str() << req.target();
        
    if (req.target() == "/"sv) {
      path << "index.html"sv;
    }

    path << '\0';

    if (!is_sub_path(path.str(), config::get().server.www_root)) {
      return response::make(response::BadRequest<ct::text_plain>(req.version(), req.keep_alive()));
    } 
    
    if (!fs::exists(path.str()) || fs::is_directory(path.str())) {
      return response::make(response::NotFound<ct::text_plain>(req.version(), req.keep_alive(), "File Not Found"sv));
    }  

    req.target(path.str());      
    return response::make(response::File(req.version(), req.keep_alive(), req.target().data()));  
  };
}

} // namespace http_handler