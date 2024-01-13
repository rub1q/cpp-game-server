#include "request_handler.hpp"

namespace http_handler {

http_string_response_t RequestHandler::handle_error(std::exception_ptr eptr, std::string_view where, 
                                                    unsigned ver, bool keep_alive) const {
  if (eptr) {
    try {
      std::rethrow_exception(eptr);
    } catch (const std::exception& e) {
      LOG_ERROR << JSON_DATA(
        {"exception"s, e.what()}, 
        {"where"s, std::move(where)}
      ) 
      << "error"sv; 
    }
  }

  return response::make(response::InternalServerError<ct::text_plain>(ver, keep_alive));
}

} // namespace http_handler