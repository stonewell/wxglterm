#include "term_buffer.h"
#include "term_window.h"
#include "cap_manager.h"

DEFINE_CAP(document_home);

void document_home(term_data_context_s & term_context,
                 const term_data_param_list & params){
    (void)params;
    (void)term_context.term_buffer->MoveCurRow((uint32_t)-1,
                                               false,
                                               true);
    term_context.term_buffer->SetCol(0);
}
