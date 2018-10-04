#pragma once

#include <memory>
#include <vector>

namespace ftdgl {

class Font {
public:
    Font() = default;
    virtual ~Font() = default;

public:
    virtual bool IsSameFont(const std::string & desc) = 0;
    virtual int GetPtSize() const = 0;
    virtual float GetDescender() const = 0;
    virtual float GetAscender() const = 0;
    virtual float GetHeight() const = 0;
};

using FontPtr = std::shared_ptr<Font>;
}
