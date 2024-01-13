#include "content_type.hpp"

#include <algorithm> 
#include <unordered_map>
#include <stdexcept>

namespace content_type {

using namespace std::literals;

namespace {

const std::unordered_map<Type, std::string> types_text = {
  // text/...
  { Type::text_plain, "text/plain" },
  { Type::text_html,  "text/html" },
  { Type::text_css,   "text/css" },
  { Type::text_javascript, "text/javascript" },

  // application/...
  { Type::app_json, "application/json" },
  { Type::app_xml,  "application/xml" },
  { Type::app_octet_stream, "application/octet-stream" },

  // image/...
  { Type::img_png,  "image/png" },
  { Type::img_jpeg, "image/jpeg" }, 
  { Type::img_gif,  "image/gif" }, 
  { Type::img_bmp,  "image/bmp" }, 
  { Type::img_ico,  "image/vnd.microsoft.icon" }, 
  { Type::img_tiff, "image/tiff" }, 
  { Type::img_svg,  "image/svg+xml" }, 

  // audio/...
  { Type::audio_mpeg, "audio/mpeg" }, 
};

const std::unordered_map<std::string, Type> file_ext = {
  // text
  { "htm",  Type::text_html }, 
  { "html", Type::text_html }, 
  { "css",  Type::text_css }, 
  { "txt",  Type::text_plain },
  { "js",   Type::text_javascript },

  // application
  { "json", Type::app_json },
  { "xml",  Type::app_xml },

  // image
  { "png",  Type::img_png },
  { "jpg",  Type::img_jpeg },
  { "jpe",  Type::img_jpeg },
  { "jpeg", Type::img_jpeg },
  { "gif",  Type::img_gif },
  { "bmp",  Type::img_bmp },
  { "ico",  Type::img_ico },
  { "tiff", Type::img_tiff },
  { "tif",  Type::img_tiff },
  { "svg",  Type::img_svg },
  { "svgz", Type::img_svg },

  // audio
  { "mp3", Type::audio_mpeg },
};

} // namespace

std::string_view get_as_text(const Type type) {
  const auto it = types_text.find(type);
  if (it != types_text.cend())
    return it->second;

  throw std::invalid_argument("Unknown content type"s);
}

Type get_ext_as_type(std::string_view extension) {
  std::string ext(extension);

  std::transform(ext.begin(), ext.end(), ext.begin(), 
                 [](unsigned char c) { return std::tolower(c); });

  if (const auto idx = ext.find("."sv); idx != std::string::npos) {
    ext = ext.substr(idx+1, ext.size());
  }
  
  const auto it = file_ext.find(ext);
  if (it != file_ext.cend())
    return it->second;

  return Type::app_octet_stream;
}

} // namespace content_type 