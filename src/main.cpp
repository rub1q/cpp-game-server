#include "app.hpp"
#include "config.hpp"
#include "logger.hpp"
#include "cli.hpp"

#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
  using namespace std::literals;
  
  auto args = cli::parse(argc, argv);
  if (!args.has_value()) {
    return EXIT_FAILURE;
  }

  try {    
    std::cout << "Initializing config..."sv << std::endl; 
    config::init(std::move(args.value()));

    std::cout << "Initializing logger..."sv << std::endl;
    logger::init(config::get().log_level);

    app::App(config::get()).run();    
  } catch (const std::exception& e) {
    LOG_ERROR << JSON_DATA(
      {"code"sv, EXIT_FAILURE},
      {"exception"sv, e.what()} 
    )
    << "server exited"sv;

    return EXIT_FAILURE;
  } catch (...) {
    LOG_ERROR << JSON_DATA(
      {"code"sv, EXIT_FAILURE},
      {"exception"sv, "Unknown error"sv} 
    )
    << "server exited"sv;
    
    return EXIT_FAILURE;
  }

  LOG_INFO 
    << JSON_DATA({"code"sv, EXIT_SUCCESS}) 
    << "server exited"sv;

  return EXIT_SUCCESS;
}