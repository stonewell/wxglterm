#include "plugin_manager.h"
#include "plugin.h"

#include "term_selection.h"
#include "term_line.h"
#include "term_cell.h"
#include "term_buffer.h"

#include "default_term_buffer.h"
#include "internal_term_buffer.h"
#include "default_term_buffer_decl.h"

TermBufferPtr CreateDefaultTermBuffer() {
    return TermBufferPtr { new DefaultTermBuffer() };
}

extern "C"
void register_plugins(PluginManagerPtr plugin_manager) {
    plugin_manager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateDefaultTermBuffer()));
}
