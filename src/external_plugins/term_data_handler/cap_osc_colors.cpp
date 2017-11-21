#include <pybind11/embed.h>

#include "term_buffer.h"
#include "term_window.h"
#include "term_network.h"
#include "cap_manager.h"
#include "term_cell.h"

#include <string.h>
#include <iostream>

DEFINE_CAP(osc_color_request);

static
uint32_t get_color_from_index(term_data_context_s & term_context,
                              int idx) {
    switch(idx) {
    case 10: //foreground
        return term_context.term_window->GetColorByIndex(TermCell::DefaultForeColorIndex);
    case 11: //background
        return term_context.term_window->GetColorByIndex(TermCell::DefaultBackColorIndex);
    case 12: //cursor
        return term_context.term_window->GetColorByIndex(TermCell::DefaultCursorColorIndex);
    default:
        return 0;
    }
}

void osc_color_request(term_data_context_s & term_context,
                       const term_data_param_list & params){
    if (params[0] >= 10 && params[0] <= 19) {
        if (!strncmp(params[1], "?", 1)) {
            uint32_t color = get_color_from_index(term_context,
                                                  params[0]);

            char rgb_response[256] {};

            sprintf(rgb_response,
                    "\033]11;rgb:%04x/%04x/%04x\007",
                    (color >> 16) & 0xFF,
                    (color >> 8) & 0xFF,
                    color & 0xFF);

            send(term_context, rgb_response);
        }
    }
}
