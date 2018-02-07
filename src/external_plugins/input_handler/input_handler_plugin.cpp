#include <pybind11/embed.h>

#include <iostream>
#include <unistd.h>
#include <vector>

#include <string.h>

#include "plugin_manager.h"
#include "plugin.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"
#include "term_window.h"
#include "input.h"
#include "plugin_base.h"

#include "app_config_impl.h"

class DefaultInputHandler
        : public virtual PluginBase
        , public virtual InputHandler
{
public:
    DefaultInputHandler() :
        PluginBase("default_input_handler", "default input handler plugin for keyboard and mouse", 1)
    {
    }

    virtual ~DefaultInputHandler() = default;

    MultipleInstancePluginPtr NewInstance() override {
        return MultipleInstancePluginPtr{new DefaultInputHandler};
    }

private:
protected:
public:
    bool ProcessKey(InputHandler::KeyCodeEnum keycode, InputHandler::ModifierEnum modifier, bool down) override {
        (void)keycode;
        (void)modifier;
        (void)down;
        return false;
    }
    bool ProcessCharInput(int32_t c, InputHandler::ModifierEnum modifier) override {
        (void)c;
        (void)modifier;
        return false;
    }
    bool ProcessMouseButton(InputHandler::MouseButtonEnum btn, uint32_t x, uint32_t y, InputHandler::ModifierEnum modifier, bool down) override {
        (void)btn;
        (void)x;
        (void)y;
        (void)modifier;
        (void)down;
        return false;
    }

    bool ProcessMouseMove(InputHandler::MouseButtonEnum btn, uint32_t x, uint32_t y, InputHandler::ModifierEnum modifier) override {
        (void)btn;
        (void)x;
        (void)y;
        (void)modifier;
        return false;
    }
private:
};

extern "C"
void register_plugins(PluginManagerPtr plugin_manager) {
    plugin_manager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(InputHandlerPtr {new DefaultInputHandler}));
}
