#pragma once 

#include "config.hpp"
#include "mux.hpp"

#include <thread>

namespace app {

class App {
public:
  App(const config::AppConfig& cfg)
    : cfg_(cfg) {
  }
  
  App(const App&) = delete;
  App& operator=(const App&) = delete;
  
  void run();

private:
  template <typename Fn>
  void run_threads(const unsigned num, const Fn& fn) const;

  mux::Router get_router() const;

private: 
  const config::AppConfig& cfg_;
  std::stop_source stop_source_;
};

} // namespace app