#include "cli.hpp"

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <iostream>

namespace cli {

using namespace std::literals;

std::optional<Args> parse(int argc, char** const argv) {
  namespace po = boost::program_options;

  Args args;

  try {
    po::options_description desc{"Allowed options"s};

    std::size_t tick, save_state_period;
    fs::path state_file_path;

    desc.add_options()
      (
        "help,h", 
        "show help"
      )
      (
        "tick-period,t", 
        po::value(&tick)->value_name("milliseconds"), 
        "set tick period"
      )
      (
        "config-file,c", 
        po::value(&args.config_file)->value_name("file"), 
        "set config file path"
      )
      (
        "www-root,w", 
        po::value(&args.www_root)->value_name("dir"), 
        "set static files root")
      (
        "randomize-spawn-points", 
        "spawn characters at random positions"
      )
      (
        "state-file", 
        po::value(&state_file_path)->value_name("file"), 
        "set file path to which application will save its state"
      )
      (
        "save-state-period", 
        po::value(&save_state_period)->value_name("save period (ms)"), 
        "set a period for automatic game state saving"
      );

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.contains("help")) {
      std::cout << desc;
      return std::nullopt;
    }

    if (!vm.contains("config-file")) {
      throw std::runtime_error("Config file path does not specified"s);
    }

    if (!vm.contains("www-root")) {
      throw std::runtime_error("Root path directory does not specified"s);
    }

    args.randomize_spawn = vm.contains("randomize-spawn-points");

    if (vm.contains("tick-period")) {
      args.tick_period = tick;
    }

    if (vm.contains("state-file")) {
      args.state_file = state_file_path;
    }

    if (vm.contains("save-state-period") && vm.contains("state-file")) {
      args.save_state_period = save_state_period;
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return std::nullopt;
  }

  return args;
}

} // namespace cli 