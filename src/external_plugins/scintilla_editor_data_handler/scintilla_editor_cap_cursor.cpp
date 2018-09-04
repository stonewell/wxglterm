#include "term_buffer.h"
#include "term_window.h"
#include "cap_manager.h"

DEFINE_CAP(document_home);
DEFINE_CAP(document_set_property);

void document_home(term_data_context_s & term_context,
                 const term_data_param_list & params){
    (void)params;
    (void)term_context.term_buffer->MoveCurRow((uint32_t)-1,
                                               false,
                                               true,
                                               term_context.cell_template);
    term_context.term_buffer->SetCol(0);
}

void document_set_property(term_data_context_s & term_context,
                           const term_data_param_list & params){
    term_context.term_buffer->SetProperty(params[0].str_value,
                                          params[1].str_value);
}
