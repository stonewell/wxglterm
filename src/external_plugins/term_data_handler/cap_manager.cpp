#include <term_buffer.h>
#include <term_window.h>
#include <cap_manager.h>
#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator

#include <unordered_map>

using cap_func_t = std::function<void(term_data_context_s & , const std::vector<int>)>;
using cap_map_t = std::unordered_map<std::string, cap_func_t>;

cap_map_t CapFuncs {
};

term_data_context_s::term_data_context_s() {
    in_status_line = false;
}

void handle_cap(term_data_context_s & term_context, const std::string & cap_name, const std::vector<int> params) {
    cap_map_t::iterator it = CapFuncs.find(cap_name);

    if (it != CapFuncs.end()) {
        it->second(term_context, params);
    } else {
        std::cerr << "unknown cap found:" << cap_name << ", with params:[";
        std::copy(params.begin(), params.end(), std::ostream_iterator<int>(std::cerr, ","));
        std::cerr << "]" << std::endl;
    }
}

void output_char(term_data_context_s & term_context, const std::string & data) {
    if (!term_context.in_status_line) {
        (void)data;
    }
}
