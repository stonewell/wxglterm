#include "plugin_manager.h"
#include "plugin.h"

#include "term_ui.h"
#include "task.h"
#include "term_gl_ui.h"

extern "C"
void register_plugins(PluginManagerPtr plugin_manager) {
    plugin_manager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateOpenGLTermUI()));
   }
