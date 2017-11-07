#pragma once

#include <functional>
#include <string>
#include <vector>

struct term_data_context_s {
    term_data_context_s();
    TermWindowPtr term_window;
    TermBufferPtr term_buffer;

    bool in_status_line;
};

void handle_cap(term_data_context_s & term_context, const std::string & cap_name, const std::vector<int> params);
void output_char(term_data_context_s & term_context, const std::string & data);
