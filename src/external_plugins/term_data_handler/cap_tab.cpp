#include "term_buffer.h"
#include "term_window.h"
#include "term_cell.h"

#include "cap_manager.h"

DEFINE_CAP(clear_tab);
DEFINE_CAP(clear_all_tabs);
DEFINE_CAP(set_tab);
DEFINE_CAP(tab);

void clear_tab(term_data_context_s & term_context,
          const term_data_param_list & params) {
    int action = 0;

    if (params.size() > 0)
        action = params[0];

    if (action == 0) {
        term_context.tab_stops.erase(term_context.term_buffer->GetCol());
    } else if (action == 3) {
        term_context.tab_stops.clear();
    }
}

void clear_all_tabs(term_data_context_s & term_context,
          const term_data_param_list & params) {
    (void)params;
    term_context.tab_stops.clear();
}

void set_tab(term_data_context_s & term_context,
          const term_data_param_list & params) {
    (void)params;
    term_context.tab_stops.emplace(term_context.term_buffer->GetCol(), true);
}

void tab(term_data_context_s & term_context,
          const term_data_param_list & params) {
    (void)params;

    auto col = term_context.term_buffer->GetCol();

    for(auto c = col + 1; c < TAB_MAX; c++) {
        if (term_context.tab_stops.find(c) != term_context.tab_stops.end()) {
            col = c;
            break;
        }
    }

    if (col >= term_context.term_buffer->GetCols())
        col = term_context.term_buffer->GetCols() - 1;

    term_context.term_buffer->SetCol(col);
}
