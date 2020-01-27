#include "plugin_manager.h"
#include "plugin.h"

#include "term_selection.h"
#include "term_line.h"
#include "term_cell.h"
#include "term_buffer.h"

#include "internal_term_shmem_buffer.h"
#include "term_shmem_buffer_decl.h"

#include "term_shmem_buffer_export.h"

TermBufferPtr CreateTermShmemBuffer() {
    return TermBufferPtr { new TermShmemBuffer() };
}

extern "C"
void TERM_SHMEM_BUFFER_EXPORT register_plugins(PluginManagerPtr plugin_manager) {
    plugin_manager->RegisterPlugin(std::dynamic_pointer_cast<Plugin>(CreateTermShmemBuffer()));
}
