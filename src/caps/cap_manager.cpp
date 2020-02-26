#include "term_buffer.h"
#include "term_window.h"
#include "cap_manager.h"

#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include <locale>
#include <codecvt>
#include <sstream>
#include <cassert>

#include <string.h>

#include "char_width.h"
#include "string_utils.h"
#include "utf8_util.h"

static
std::wstring_convert<std::codecvt_utf8<wchar_t
#if defined(_WIN32) && defined(__GNUC__)
                                       , 0x10ffff, std::little_endian
#endif
                                       >, wchar_t> wcharconv;

DEFINE_CAP(bell);
DEFINE_CAP(to_status_line);
DEFINE_CAP(from_status_line);
DEFINE_CAP(meta_on);
DEFINE_CAP(operating_system_control)

using cap_map_t = std::unordered_map<std::string, cap_func_t>;

static std::shared_ptr<cap_map_t> CapFuncs {nullptr};

term_cap_s::term_cap_s(const std::string & name,
                       cap_func_t cap_func) {
    if (!CapFuncs)
        CapFuncs = std::make_shared<cap_map_t>();

    CapFuncs->emplace(name, cap_func);
}

term_data_context_s::term_data_context_s():
    in_status_line(false)
    , auto_wrap(true)
    , origin_mode(false)
    , dec_mode(false)
    , force_column(false)
    , keypad_transmit_mode(false)
    , force_column_count(80)
    , saved_col((uint32_t)-1)
    , saved_row((uint32_t)-1)
    , remain_buffer {}
    , remain_buffer_index {0}
    , charset_mode {0}
    , charset_modes_translate {nullptr, nullptr}
    , saved_charset_mode {0}
    , saved_charset_modes_translate {nullptr, nullptr}
    , saved_origin_mode {false}
    , cap_debug {false}
    , cell_motion_mouse_track {false}
    , sgr_mouse_mode {false}
    , send_focus_in_out {false}
{

    for(int i = 0; i < TAB_MAX; i += TAB_WIDTH) {
        tab_stops.emplace(i, true);
    }
}

void handle_cap(term_data_context_s & term_context, const std::string & cap_name, const term_data_param_list params) {
    cap_map_t::iterator it = CapFuncs->find(cap_name);

    if (term_context.cap_debug) {
        std::cerr << "handle cap:" << cap_name;
        std::cerr << ",params:[";
        std::cerr << join(params, ",");
        std::cerr << "]" << std::endl;
    }

    if (it != CapFuncs->end()) {
        it->second(term_context, params);
    } else {
        std::cerr << "unknown cap found:" << cap_name << ", with params:["
                  << join(params, ",");
        std::cerr << "]" << std::endl;
    }
}

void output_char(term_data_context_s & term_context, char data, bool insert) {
    if (term_context.in_status_line) {
        (void)data;
    } else {
        term_context.remain_buffer[term_context.remain_buffer_index++] = data;

        wchar_t codepoint = 0;
        size_t converted = utf8_to_wchar(term_context.remain_buffer,
                                         term_context.remain_buffer_index,
                                         codepoint);

        if (converted == 0)
            return;

        assert((size_t)term_context.remain_buffer_index == converted);

        term_context.remain_buffer_index = 0;

        if (term_context.charset_modes_translate[term_context.charset_mode]) {
            codepoint = term_context.charset_modes_translate[term_context.charset_mode](codepoint);
        }

        auto width = char_width(codepoint);

        if (width == 0) {
            return;
        }

        if (!term_context.auto_wrap
            && term_context.term_buffer->GetCol() >= term_context.term_buffer->GetCols())
            term_context.term_buffer->SetCol(term_context.term_buffer->GetCols() - 1);

        term_context.term_buffer->SetCurCellData(static_cast<uint32_t>(codepoint),
                                                 width > 1,
                                                 insert,
                                                 term_context.cell_template);
    }
}

void bell(term_data_context_s & term_context,
          const term_data_param_list & params) {
    (void)params;
    term_context.in_status_line = false;
}

void to_status_line(term_data_context_s & term_context,
                    const term_data_param_list & params) {
    (void)params;
    term_context.in_status_line = true;
}

void from_status_line(term_data_context_s & term_context,
                      const term_data_param_list & params) {
    (void)params;
    term_context.in_status_line = false;
}

void meta_on(term_data_context_s & term_context,
             const term_data_param_list & params) {
    (void)term_context;
    (void)params;
}

void resize_terminal(term_data_context_s & term_context) {
    auto rows = term_context.term_buffer->GetRows();
    auto cols = term_context.term_buffer->GetCols();

    if (term_context.force_column) {
        rows = 25;
        cols = term_context.force_column_count;
    }

    term_context.term_buffer->Resize(rows, cols);
    term_context.term_buffer->SetScrollRegionBegin(0);
    term_context.term_buffer->SetScrollRegionEnd(rows - 1);
}

void set_cursor(term_data_context_s & term_context,
                uint32_t col,
                uint32_t row) {
    auto rows = term_context.term_buffer->GetRows();
    auto cols = term_context.term_buffer->GetCols();
    auto end = rows - 1;

    if (term_context.origin_mode) {
        auto begin = term_context.term_buffer->GetScrollRegionBegin();
        end = term_context.term_buffer->GetScrollRegionEnd();
        row += begin;
    }

    if (row > end)
        row = end;

    if (col > cols)
        col = cols - 1;

    term_context.term_buffer->SetCol(col);
    term_context.term_buffer->SetRow(row);
}

void operating_system_control(term_data_context_s & term_context,
                              const term_data_param_list & params)
{
    if (params[0] >= 0 && params[0] <= 3) {
        if (term_context.cap_debug) {
            std::cerr << "handle status line:"
                      << params[0]
                      << ",params:["
                      << params[1]
                      << "]" << std::endl;
        }

        set_window_properties(term_context,
                              params[0],
                              params[1].str_value);
        return;
    } else if (params[0] >= 10 && params[0] <= 19) {
        handle_cap(term_context, "osc_color_request", params);
        return;
    } else if (params[0] == 52) {
        handle_cap(term_context, "osc_selection_request", params);
        return;
    }


    std::cerr << "unimplemented operating system control:" << params[0] << ", with params:["
              << join(params.begin() + 1, params.end(), "");
    std::cerr << "]" << std::endl;
}

void operating_system_control_0(term_data_context_s & term_context,
                                const term_data_param_list & params)
{
    std::string ss = join(params, "");

    if (term_context.cap_debug) {
        std::cerr << "handle status line:"
                  << 0
                  << ",params:["
                  << ss
                  << "]" << std::endl;
    }

    set_window_properties(term_context,
                          0,
                          ss);
}
