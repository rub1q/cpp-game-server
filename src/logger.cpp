#include "logger.hpp"

#include <boost/log/utility/setup/console.hpp>
#include <syncstream>

namespace logger {

namespace keywords = log::keywords;
namespace json = boost::json;

using namespace std::literals;

namespace {

std::unordered_map<std::string, log::trivial::severity_level> levels = {
  { "TRACE"s, log::trivial::trace },
  { "DEBUG"s, log::trivial::debug },
  { "INFO"s,  log::trivial::info },
  { "WARN"s,  log::trivial::warning },
  { "ERROR"s, log::trivial::error },
  { "FATAL"s, log::trivial::fatal },
};

void formatter(const log::record_view& rec, log::formatting_ostream& strm) {
  json::object obj;
  
  obj["timestamp"] = to_iso_extended_string(boost::posix_time::microsec_clock::local_time());
  obj["data"]      = rec[data].empty() ? "" : rec[data].get();
  obj["message"]   = rec[log::expressions::smessage].get();

  std::osyncstream(strm.stream()) << json::serialize(obj); 
}

} // namespace

void init(std::string_view log_level) {  
  const auto it = levels.find(std::string(log_level));
  if (it == levels.cend()) {
    throw std::invalid_argument("invalid log level. Expect: TRACE, DEBUG, INFO, WARN, ERROR, FATAL"s);
  }

  log::add_console_log(
    std::cout, 
    keywords::format = &formatter,
    keywords::filter = log::trivial::severity >= it->second,
    keywords::auto_flush = true
  );
}

} // namespace logger