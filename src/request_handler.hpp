#pragma once

#include "listener.hpp"
#include "mux.hpp"
#include "logger.hpp"

#include <chrono>

namespace http_handler {

using namespace common;
using namespace std::literals;
using namespace std::chrono;

namespace ct = content_type;

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:
  using Strand = http_server::net::strand<http_server::net::io_context::executor_type>;

  explicit RequestHandler(Strand api_strand, mux::Router&& router)
    : api_strand_{api_strand}
    , router_(std::move(router)) {
  }

  RequestHandler(const RequestHandler&) = delete;
  RequestHandler& operator=(const RequestHandler&) = delete;

  template <typename Body, typename Allocator, typename Send>
  http_response_t operator()(http_request_t<Body, Allocator>&& req, Send&& send) { 
    if (req.target().starts_with("/api/"sv))
      return handle_api_request(std::move(req), std::forward<decltype(send)>(send));

    return handle_file_request(std::move(req), std::forward<decltype(send)>(send));
  }

private:
  http_string_response_t handle_error(std::exception_ptr eptr, std::string_view where, 
                                      unsigned ver, bool keep_alive) const;

  template <typename Body, typename Allocator, typename Send>
  http_response_t handle_api_request(http_request_t<Body, Allocator>&& req, Send&& send) {
    auto match = router_.process(req);
    
    if (match.error == mux::MatchError::NotFound && !match.handler) {
      auto resp = response::make(response::BadRequest<ct::app_json>(req.version(), req.keep_alive())
        .add_body(response::basic_json_body::bad_request()));

      http_response_t saved_response = resp;
      send(std::move(resp));

      return saved_response;
    }

    return std::visit([self = shared_from_this(), &req, send = std::forward<decltype(send)>(send)](auto&& response) {
      http_response_t saved_response = static_cast<http_string_response_t>(response);
      
      try {            
        auto handle = [self, &req, send = std::forward<decltype(send)>(send), &response, &saved_response] {
          try {
            assert(self->api_strand_.running_in_this_thread());
            send(std::forward<decltype(response)>(response));
          } catch (...) {
            auto err_resp = self->handle_error(std::current_exception(), __FUNCTION__, req.version(), req.keep_alive());
            saved_response = err_resp;
    
            send(std::move(err_resp)); 
          }
        };

        http_server::net::dispatch(self->api_strand_, handle);
      } catch (...) {
        auto err_resp = self->handle_error(std::current_exception(), __FUNCTION__, req.version(), req.keep_alive());
        saved_response = err_resp;

        send(std::move(err_resp)); 
      }

      return saved_response;
    }, 
    (*match.handler)(std::move(req))); 
  }

  template <typename Body, typename Allocator, typename Send>
  http_response_t handle_file_request(http_request_t<Body, Allocator>&& req, Send&& send) {  
    auto match = router_.process(req);
    
    return std::visit([self = shared_from_this(), req, send = std::forward<decltype(send)>(send)](auto&& response) {
      http_response_t saved_response = static_cast<http_string_response_t>(response);
      
      try {            
        send(std::forward<decltype(response)>(response));
      } catch (...) {
        auto err_resp = self->handle_error(std::current_exception(), __FUNCTION__, req.version(), req.keep_alive());
        saved_response = err_resp;

        send(std::move(err_resp)); 
      }

      return saved_response;
    }, 
    (*match.handler)(std::move(req)));
  }

private:
  mux::Router router_;
  Strand api_strand_;
};

template <typename RequestHandler>
class LoggingRequestHandler {
public:
  LoggingRequestHandler(RequestHandler&& handler)
    : handler_(std::move(handler)) {
  }

  template <typename Body, typename Allocator, typename Send>
  void operator()(http_server::tcp::endpoint remote_endpoint, http_request_t<Body, Allocator>&& req, Send&& send) {
    LOG_REQUEST(remote_endpoint.address().to_string(), req.target(), req.method_string())

    const auto request_time  = steady_clock::now(); 
    const auto response = handler_(std::move(req), std::forward<decltype(send)>(send));
    const auto response_time = duration_cast<milliseconds>(steady_clock::now() - request_time);

    return std::visit([&response_time](auto&& response) {
      LOG_RESPONSE(response_time.count(), response.result_int(), response[http::field::content_type])
    }, 
    response);   
  }

private:
  RequestHandler handler_;
};

}  // namespace http_handler