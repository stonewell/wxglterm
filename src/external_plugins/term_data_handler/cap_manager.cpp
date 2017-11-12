#include "term_buffer.h"
#include "term_window.h"
#include "cap_manager.h"

#include <iostream>
#include <algorithm> // for copy
#include <iterator> // for ostream_iterator
#include <locale>
#include <codecvt>
#include <unordered_map>

#include "char_width.h"

using cap_map_t = std::unordered_map<std::string, cap_func_t>;

static
std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcharconv;

static
cap_map_t CapFuncs {
};

DEFINE_CAP(bell);
DEFINE_CAP(to_status_line);
DEFINE_CAP(from_status_line);

term_cap_s::term_cap_s(const std::string & name,
                   cap_func_t cap_func) {
    CapFuncs.emplace(name, cap_func);
}

term_data_context_s::term_data_context_s():
    in_status_line(false)
    , auto_wrap(false)
    , origin_mode(false) {
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

void output_char(term_data_context_s & term_context, const std::string & data, bool insert) {
    if (term_context.in_status_line) {
        (void)data;
    } else {
        term_context.remain_buffer.insert(term_context.remain_buffer.end(),
                                          data.begin(),
                                          data.end());

        std::wstring w_str;
        size_t converted = 0;
        try {
            w_str = wcharconv.from_bytes(&term_context.remain_buffer[0],
                                         &term_context.remain_buffer[term_context.remain_buffer.size() - 1] + 1);
            converted = wcharconv.converted();
        } catch(const std::range_error& e) {
            converted = wcharconv.converted();
            if (converted == 0)
                return;
            w_str = wcharconv.from_bytes(&term_context.remain_buffer[0],
                                         &term_context.remain_buffer[converted]);
        }

        if (w_str.length() == 0)
            return;

        term_context.remain_buffer.erase(term_context.remain_buffer.begin(),
                                         term_context.remain_buffer.begin() + converted);

        for (auto it : w_str){
            auto width = char_width(it);

            if (width == 0) {
                continue;
            }

            if (!term_context.auto_wrap && term_context.term_buffer->GetCol() >= term_context.term_buffer->GetCols())
                term_context.term_buffer->SetCol(term_context.term_buffer->GetCols() - 1);

            term_context.term_buffer->SetCurCellData(static_cast<uint32_t>(it),
                                                     width > 1,
                                                     insert,
                                                     term_context.cell_template);
        }
    }
}

void bell(term_data_context_s & term_context,
          const std::vector<int> & params) {
    (void)params;
    term_context.in_status_line = false;
}

void to_status_line(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    (void)params;
    term_context.in_status_line = true;
}

void from_status_line(term_data_context_s & term_context,
                    const std::vector<int> & params) {
    (void)params;
    term_context.in_status_line = false;
}
