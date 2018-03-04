#include "term_buffer.h"
#include "term_window.h"
#include "term_cell.h"

#include "cap_manager.h"

DEFINE_CAP(change_scroll_region);
DEFINE_CAP(change_scroll_region_from_start);
DEFINE_CAP(change_scroll_region_to_end);

void change_scroll_region(term_data_context_s & term_context,
                    const term_data_param_list & params) {

    if (params.size() == 0) {
        term_context.term_buffer->SetScrollRegionBegin(0);
        term_context.term_buffer->SetScrollRegionEnd(0);
    } else {
        term_context.term_buffer->SetScrollRegionBegin(params[0]);
        term_context.term_buffer->SetScrollRegionEnd(params[1]);
    }
}

void change_scroll_region_from_start(term_data_context_s & term_context,
                    const term_data_param_list & params) {
    term_context.term_buffer->SetScrollRegionBegin(0);
    term_context.term_buffer->SetScrollRegionEnd(params[0]);
    handle_cap(term_context, "cursor_home", params);
}

void change_scroll_region_to_end(term_data_context_s & term_context,
                    const term_data_param_list & params) {
    term_context.term_buffer->SetScrollRegionBegin(params[0]);
    term_context.term_buffer->SetScrollRegionEnd(term_context.term_buffer->GetRows() - 1);
    handle_cap(term_context, "cursor_home", params);
}
