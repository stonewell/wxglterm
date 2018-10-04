#pragma once

#include "font.h"
#include <string>

namespace fttb {
namespace impl {
FontPtr CreateFontFromDesc(FT_Library & library, const std::string & desc, float dpi, float dpi_height);
} //namespace impl
} //namespace fttb
