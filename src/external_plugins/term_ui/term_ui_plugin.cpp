#include "plugin_manager.h"
#include "plugin.h"

#include "term_ui.h"
#include "task.h"

#include "default_term_ui.h"
#include "term_ui_export.h"

extern "C"
void TERM_UI_EXPORT register_plugins(PluginManagerPtr plugin_manager) {
    plugin_manager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateDefaultTermUI()));
}
