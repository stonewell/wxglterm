#include <pybind11/embed.h>

#include <iostream>
#include <unistd.h>
#include <vector>

#include <string.h>

#include "key_code_map.h"

#include "plugin_manager.h"
#include "plugin.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"
#include "term_window.h"
#include "input.h"
#include "plugin_base.h"

#include "app_config_impl.h"
#include "input_handler_plugin.h"

extern "C"
void register_plugins(PluginManagerPtr plugin_manager) {
    plugin_manager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(InputHandlerPtr {new DefaultInputHandler}));
}
