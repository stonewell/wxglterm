#include <iostream>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#endif

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

#include "term_input_handler_export.h"

extern "C"
void TERM_INPUT_HANDLER_EXPORT register_plugins(PluginManagerPtr plugin_manager) {
    plugin_manager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(InputHandlerPtr {new DefaultInputHandler}));
}
