#pragma once

#include "multiple_instance_plugin.h"

class TermColor {
public:
    TermColor() :
        r(0)
        , g(0)
        , b(0)
        , a(0xFF) {
    }

    virtual ~TermColor() = default;

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

class TermColorTheme : public virtual MultipleInstancePlugin  {
public:
    virtual bool Load(const char * name) = 0;
    virtual bool LoadWithValues(const char * name, const char * value) = 0;

    //when index > TermCell::ColorIndexCount, it is real rgb value
    // the rgb value = index - TermCell::ColorIndexCount
    // it rgb value only, no alpha
    virtual TermColorPtr GetColor(uint32_t index) = 0;
    virtual void SetColor(uint32_t index, TermColorPtr c) = 0;
};
