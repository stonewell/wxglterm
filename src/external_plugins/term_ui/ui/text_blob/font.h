#pragma once

#include <memory>
#include <vector>

namespace fttb {

class Font {
public:
    Font() = default;
    virtual ~Font() = default;

public:
    virtual bool IsSameFont(const std::string & desc) = 0;
};

using FontPtr = std::shared_ptr<Font>;
}
