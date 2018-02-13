#include "plugin_manager.h"
#include "plugin.h"

#include "term_selection.h"
#include "term_line.h"
#include "term_cell.h"
#include "term_buffer.h"

#include "scintilla_editor.h"

TermBufferPtr CreateDefaultTermBuffer() {
    return TermBufferPtr { new ScintillaEditor() };
}

extern "C"
void register_plugins(PluginManagerPtr plugin_manager) {
    plugin_manager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateDefaultTermBuffer()));
}
