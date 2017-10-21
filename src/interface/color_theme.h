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

    virtual TermColorPtr GetColor(uint8_t index) = 0;
};
