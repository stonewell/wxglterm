#include <pybind11/embed.h>

#include "term_buffer.h"
#include "term_window.h"
#include "term_network.h"
#include "cap_manager.h"
#include "term_cell.h"
#include "string_utils.h"

#include <string.h>
#include <iostream>
#include <sstream>
#include <iterator>

DEFINE_CAP(osc_selection_request);

void osc_selection_request(term_data_context_s & term_context,
                           const term_data_param_list & params){
    if (params[0] == 52) {
        std::string buffer_index = "0";
        std::string ss_data;

        std::string v = params[1].str_value;

        auto pos = v.find(";");

        if (pos != std::string::npos) {
            buffer_index = v.substr(0, pos);
            ss_data = v.substr(pos + 1);
        }

        if (ss_data == "?") {
            std::string sel_data = term_context.term_window->GetSelectionData();

            std::stringstream ss;

            ss << "\033]52;" << buffer_index << ";"
               << sel_data << "\007";

            send(term_context, ss.str().c_str());
        } else{
            term_context.term_window->SetSelectionData(ss_data);
        }
    }
}
