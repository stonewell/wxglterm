#pragma once

#include "input.h"
#include "py_multiple_instance_plugin.h"

template<class InputPluginBase = InputPlugin>
class PyInputPlugin : public virtual PyMultipleInstancePlugin<InputPluginBase> {
public:
    using PyMultipleInstancePlugin<InputPluginBase>::PyMultipleInstancePlugin;

    bool ProcessKey(InputPlugin::KeyCodeEnum keycode, InputPlugin::ModifierEnum modifier, bool down) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, InputPluginBase, "process_key", ProcessKey, keycode, modifier, down);
    }
    bool ProcessCharInput(int32_t c, InputPlugin::ModifierEnum modifier) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, InputPluginBase, "process_char_input", ProcessCharInput, c, modifier);
    }
    bool ProcessMouseButton(InputPlugin::MouseButtonEnum btn, uint32_t x, uint32_t y, InputPlugin::ModifierEnum modifier, bool down) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, InputPluginBase, "process_mouse_button", ProcessMouseButton, btn, x, y, modifier, down);
    }

    bool ProcessMouseMove(InputPlugin::MouseButtonEnum btn, uint32_t x, uint32_t y, InputPlugin::ModifierEnum modifier) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, InputPluginBase, "process_mouse_move", ProcessMouseMove, btn, x, y, modifier);
    }
};
