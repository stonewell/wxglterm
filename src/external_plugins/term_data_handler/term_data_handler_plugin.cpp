#include <iostream>
#include "plugin_manager.h"
#include "term_data_handler_impl.h"
#include "term_data_handler_export.h"

extern "C"
void TERM_DATA_HANDLER_EXPORT register_plugins(PluginManagerPtr plugin_manager) {
    (void)plugin_manager;
    plugin_manager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateTermDataHandler()));
}
