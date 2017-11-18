#pragma once

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

using charset_mode_func_t = std::function<wchar_t(wchar_t)>;

struct term_data_context_s {
    term_data_context_s();

    TermWindowPtr term_window;
    TermBufferPtr term_buffer;
    TermNetworkPtr term_network;
    TermCellPtr cell_template;
    TermCellPtr saved_cell_template;
    TermCellPtr default_cell_template;

    bool in_status_line;
    bool auto_wrap;
    bool origin_mode;
    bool dec_mode;
    bool force_column;
    bool keypad_transmit_mode;

    uint32_t force_column_count;
    uint32_t saved_col;
    uint32_t saved_row;

    std::vector<char> remain_buffer;
    std::unordered_map<uint32_t, bool> tab_stops;

    int charset_mode;
    charset_mode_func_t charset_modes_translate[2];
    int saved_charset_mode;
    charset_mode_func_t saved_charset_modes_translate[2];

    bool saved_origin_mode;
    bool cap_debug;
};

struct term_data_param_s {
    std::string str_value;
    int int_value;

    operator int () const {
        return int_value;
    }

    operator const char * () const {
        return str_value.c_str();
    }

    friend std::ostream& operator<<(std::ostream& os, const term_data_param_s& dt) {
        os << "[int:" << dt.int_value << ", str:" << dt.str_value << "]";
        return os;
    }
};

using term_data_param_list = std::vector<term_data_param_s>;

#define TAB_MAX (999)
#define TAB_WIDTH (8)

using cap_func_t = std::function<void(term_data_context_s & , const term_data_param_list &)>;

struct term_cap_s {
    term_cap_s(const std::string & name,
             cap_func_t cap_func);
};

#define DEFINE_CAP(cap) \
    static void cap(term_data_context_s & term_context, const term_data_param_list& params);\
    static term_cap_s __##cap{ #cap, cap};

void handle_cap(term_data_context_s & term_context, const std::string & cap_name, const term_data_param_list params);
void output_char(term_data_context_s & term_context, char data, bool insert);
void resize_terminal(term_data_context_s & term_context);
void set_cursor(term_data_context_s & term_context,
                uint32_t col,
                uint32_t row);
