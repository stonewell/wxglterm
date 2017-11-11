#include "term_buffer.h"
#include "term_window.h"
#include "term_cell.h"

#include "cap_manager.h"

#include <bitset>

DEFINE_CAP(set_a_foreground);
DEFINE_CAP(set_a_background);
DEFINE_CAP(origin_pair);
DEFINE_CAP(set_attributes);

constexpr int32_t VALUE_NOT_CHANGE = -1;

static
void __do_set_attributes(term_data_context_s & term_context,
                      const std::bitset<32> & mode,
                      int32_t fore_color_idx,
                      int32_t back_color_idx) {
    if (mode.test(0)) {
        term_context.cell_template->SetMode(term_context.default_cell_template->GetMode());
    } else if (mode.test(1)) {
        term_context.cell_template->AddMode(TermCell::Bold);
    } else if (mode.test(2)) {
        term_context.cell_template->AddMode(TermCell::Dim);
    } else if (mode.test(7)) {
        term_context.cell_template->AddMode(TermCell::Reverse);
    } else if (mode.test(21) || mode.test(22)) {
        term_context.cell_template->RemoveMode(TermCell::Bold);
        term_context.cell_template->RemoveMode(TermCell::Dim);
    } else if (mode.test(27)) {
        term_context.cell_template->RemoveMode(TermCell::Reverse);
    }

    if (fore_color_idx >= 0)
        term_context.cell_template->SetForeColorIndex((uint16_t)(fore_color_idx & 0xFFFF));
    if (back_color_idx >= 0)
        term_context.cell_template->SetBackColorIndex((uint16_t)(back_color_idx & 0xFFFF));
}

void set_a_foreground(term_data_context_s & term_context,
                  const std::vector<int> & params){
    bool light = false;
    uint16_t color_idx = 0;

    if (params.size() == 2) {
        light = (params[0] == 1);
        color_idx = params[1] - 30;
    } else if (params.size() == 3 && params[0] == 38 && params[1] == 5) {
        color_idx = params[2];
    } else {
        color_idx = params[0] - 30;
    }

    std::bitset<32> mode;

    if (light)
        mode.set(1);

    __do_set_attributes(term_context,
                     mode,
                     color_idx,
                     VALUE_NOT_CHANGE);
}

void set_a_background(term_data_context_s & term_context,
                  const std::vector<int> & params){
    bool light = false;
    uint16_t color_idx = 0;

    if (params.size() == 2) {
        light = (params[0] == 1);
        color_idx = params[1] - 40;
    } else if (params.size() == 3 && params[0] == 48 && params[1] == 5) {
        color_idx = params[2];
    } else {
        color_idx = params[0] - 40;
    }

    std::bitset<32> mode;

    if (light)
        mode.set(1);

    __do_set_attributes(term_context,
                     mode,
                     VALUE_NOT_CHANGE,
                     color_idx);
}

void origin_pair(term_data_context_s & term_context,
                  const std::vector<int> & params){
    (void)params;

    term_context.cell_template->SetForeColorIndex(term_context.default_cell_template->GetForeColorIndex());
    term_context.cell_template->SetBackColorIndex(term_context.default_cell_template->GetBackColorIndex());
}

void set_attributes(term_data_context_s & term_context,
                  const std::vector<int> & params){
    std::bitset<32> mode {};
    int32_t fore_color_idx = VALUE_NOT_CHANGE;
    int32_t back_color_idx = VALUE_NOT_CHANGE;

    for(int v : params) {
        if (v == 0) {
            origin_pair(term_context, params);
        } else if (v >= 1 && v <= 29) {
            mode.set(v);
        } else if (v == 39) {
            fore_color_idx = term_context.default_cell_template->GetForeColorIndex();
        } else if (v == 49) {
            back_color_idx = term_context.default_cell_template->GetBackColorIndex();
        } else if ((v >= 30 && v <=37) || (v >= 90 && v <= 97)) {
            fore_color_idx = v % 10;

            if (v >= 90)
                fore_color_idx += 8;
        } else if ((v >= 40 && v <=47) || (v >= 100 && v <= 107)) {
            back_color_idx = v % 10;

            if (v >= 90)
                back_color_idx += 8;
        }
    }

    __do_set_attributes(term_context,
                     mode,
                     fore_color_idx,
                     back_color_idx);
}
