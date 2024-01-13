#pragma once 

#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/json.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

BOOST_LOG_ATTRIBUTE_KEYWORD(data, "Data", boost::json::value)

#define JSON_DATA(...)  \
  logger::log::add_value(data, {__VA_ARGS__}) 

#define LOG(severity) \
  BOOST_LOG_TRIVIAL(severity)

#define LOG_TRACE   LOG(trace)
#define LOG_DEBUG   LOG(debug)
#define LOG_INFO    LOG(info)
#define LOG_WARN    LOG(warning)
#define LOG_ERROR   LOG(error)
#define LOG_FATAL   LOG(fatal)

#define LOG_REQUEST(ip, target, method) \
  LOG_INFO << JSON_DATA( \
    {"ip"sv, ip}, \
    {"URI"sv, target},  \
    {"method"sv, method}  \
  )  \
  << "request received"sv;

#define LOG_RESPONSE(response_time, code, content_type) \
  LOG_INFO << JSON_DATA( \
    {"response_time"sv, response_time}, \
    {"code"sv, code}, \
    {"content_type"sv, content_type}  \
  )  \
  << "response sent"sv;

#define LOG_SYSTEM_ERROR(code, text) \
  LOG_ERROR << JSON_DATA(  \
    {"code"s, code},  \
    {"text"s, text},  \
    {"where"s, __FUNCTION__}  \
  ) \
  << "error"sv;

namespace logger {

namespace log = boost::log;

void init(std::string_view log_level);

} // namespace logger