#pragma once 

#include <string>

namespace content_type {

enum Type {
  // text/...
  text_plain,
  text_html, 
  text_css,
  text_javascript,
  
  // application/...
  app_json, 
  app_xml, 
  app_octet_stream,

  // image/...
  img_png,
  img_jpeg,
  img_gif,
  img_bmp,
  img_ico,
  img_tiff,
  img_svg,

  // audio/...
  audio_mpeg
};

[[nodiscard]] std::string_view get_as_text(const Type type);
[[nodiscard]] Type get_ext_as_type(std::string_view extension);

} // namespace content_type