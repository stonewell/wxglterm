#pragma once

#include "color_theme.h"
#include "py_multiple_instance_plugin.h"

template<class TermColorThemeBase = TermColorTheme>
class PyTermColorTheme : public virtual PyMultipleInstancePlugin<TermColorThemeBase> {
public:
    using PyMultipleInstancePlugin<TermColorThemeBase>::PyMultipleInstancePlugin;

    bool Load(const char * name) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, TermColorThemeBase, "load", Load, name);
    }

    bool LoadWithValues(const char * name, const char * value) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, TermColorThemeBase, "load_with_values", LoadWithValues, name, value);
    }

    TermColorPtr GetColor(uint32_t index) override {
        PYBIND11_OVERLOAD_PURE_NAME(TermColorPtr, TermColorThemeBase, "get_color", GetColor, index);
    }
};
