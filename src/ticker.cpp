#include "ticker.hpp"
#include "logger.hpp"

#include <boost/asio/dispatch.hpp>

namespace gstime {

using namespace std::literals;

void Ticker::start() {
  net::dispatch(strand_, [self = shared_from_this()] {
    self->last_tick_ = steady_clock::now();
    self->shedule_tick();
  });
}

void Ticker::shedule_tick() {
  assert(strand_.running_in_this_thread());
  
  timer_.expires_after(period_);
  timer_.async_wait([self = shared_from_this()](sys::error_code ec) {
    self->on_tick(ec);
  });
}

void Ticker::on_tick(sys::error_code ec) {
  assert(strand_.running_in_this_thread());

  if (ec) {
    LOG_SYSTEM_ERROR(ec.value(), ec.message())
    return;
  }

  const auto tick = steady_clock::now();
  const auto delta = duration_cast<milliseconds>(tick - last_tick_);

  last_tick_ = tick;

  try {
    handler_(delta);
  } catch (...) {
    //
  }

  shedule_tick();
}

} // namespace gstime