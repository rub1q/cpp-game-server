#include "http_methods.hpp"

namespace http_methods {

using namespace std::literals;

std::unordered_map<std::string, Method> AllowedMethod::verbs_ = {
  { "GET"s,  Method::get },
  { "HEAD"s, Method::head },
  { "POST"s, Method::post },
  { "PUT"s,  Method::put },
};

[[nodiscard]] bool AllowedMethod::is_allowed(std::string_view method) const {
  const auto m = verbs_.find(std::string(method));
  return (m != verbs_.cend()) && ((allowed_methods_ & m->second) != Method::unknown);
}

void AllowedMethod::set(const Method methods) noexcept {
  allowed_methods_ = methods;
}

[[nodiscard]] std::string AllowedMethod::as_string() const {
  std::string res; 

  for (const auto& [methodStr, methodVal] : verbs_) {
    if ((methodVal & allowed_methods_) != Method::unknown) {
      if (!res.empty())
        res += ", ";

      res += methodStr;
    }
  }

  return res;
}

} // http_methods