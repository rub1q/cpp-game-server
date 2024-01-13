#pragma once

#include "common_http.hpp"

#include <cstdint>
#include <unordered_map>

namespace http_methods {

enum class Method : std::uint16_t {
  unknown = 0,
  get = 1,
  head = 1 << 1, 
  post = 1 << 2, 
  put = 1 << 3
};

} // namespace http_methods

inline constexpr http_methods::Method
operator|(http_methods::Method m1, http_methods::Method m2) {
  using ut = std::underlying_type_t<http_methods::Method>;
  return static_cast<http_methods::Method>(static_cast<ut>(m1) | static_cast<ut>(m2));
}

inline constexpr http_methods::Method
operator&(http_methods::Method m1, http_methods::Method m2) {
  using ut = std::underlying_type_t<http_methods::Method>;
  return static_cast<http_methods::Method>(static_cast<ut>(m1) & static_cast<ut>(m2));
}

namespace http_methods {  

using namespace common;
using namespace std::literals;

class AllowedMethod {
public:
  [[nodiscard]] bool is_allowed(std::string_view method) const;
  [[nodiscard]] std::string as_string() const;

  void set(const Method methods) noexcept;

private:
  Method allowed_methods_;

  static std::unordered_map<std::string, Method> verbs_;
};

} // namespace http_methods