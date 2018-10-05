#pragma once

#include <memory>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace fttb {

class Font {
public:
    Font() = default;
    virtual ~Font() = default;

public:
    virtual bool IsSameFont(const std::string & desc) = 0;
    virtual FT_Face EnsureGlyph(uint32_t codepoint) = 0;
};

using FontPtr = std::shared_ptr<Font>;
}
