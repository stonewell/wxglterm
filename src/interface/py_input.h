#pragma once

#include "input.h"
#include "py_multiple_instance_plugin.h"

template<class InputHandlerBase = InputHandler>
class PyInputHandler : public PyMultipleInstancePlugin<InputHandlerBase> {
public:
    using PyMultipleInstancePlugin<InputHandlerBase>::PyMultipleInstancePlugin;

    bool ProcessKey(InputHandler::KeyCodeEnum keycode, InputHandler::ModifierEnum modifier, bool down) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, InputHandlerBase, "process_key", ProcessKey, keycode, modifier, down);
    }
    bool ProcessCharInput(int32_t c, InputHandler::ModifierEnum modifier) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, InputHandlerBase, "process_char_input", ProcessCharInput, c, modifier);
    }
    bool ProcessMouseButton(InputHandler::MouseButtonEnum btn, uint32_t x, uint32_t y, InputHandler::ModifierEnum modifier, bool down) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, InputHandlerBase, "process_mouse_button", ProcessMouseButton, btn, x, y, modifier, down);
    }

    bool ProcessMouseMove(InputHandler::MouseButtonEnum btn, uint32_t x, uint32_t y, InputHandler::ModifierEnum modifier) override {
        PYBIND11_OVERLOAD_PURE_NAME(bool, InputHandlerBase, "process_mouse_move", ProcessMouseMove, btn, x, y, modifier);
    }
};
