#pragma once 

#include <boost/beast/http.hpp>
#include <variant>

namespace common {

namespace beast = boost::beast;
namespace http = beast::http;

using http_string_response_t = http::response<http::string_body>;
using http_file_response_t = http::response<http::file_body>;
using http_string_request_t = http::request<http::string_body>;

using http_response_t = std::variant<http_string_response_t, http_file_response_t>;  

template <typename Body, typename Allocator>
using http_request_t = http::request<Body, http::basic_fields<Allocator>>;

} // namespace common