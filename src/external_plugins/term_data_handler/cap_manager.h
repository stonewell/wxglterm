#pragma once

#include <functional>
#include <string>
#include <vector>

struct term_data_context_s {
    term_data_context_s();

    TermWindowPtr term_window;
    TermBufferPtr term_buffer;
    TermCellPtr cell_template;
    TermCellPtr default_cell_template;

    bool in_status_line;
    bool auto_wrap;
    std::vector<char> remain_buffer;
};

using cap_func_t = std::function<void(term_data_context_s & , const std::vector<int> &)>;

struct term_cap_s {
    term_cap_s(const std::string & name,
             cap_func_t cap_func);
};

#define DEFINE_CAP(cap) \
    static void cap(term_data_context_s & term_context, const std::vector<int>& params);\
    static term_cap_s __##cap{ #cap, cap};

void handle_cap(term_data_context_s & term_context, const std::string & cap_name, const std::vector<int> params);
void output_char(term_data_context_s & term_context, const std::string & data, bool insert);
