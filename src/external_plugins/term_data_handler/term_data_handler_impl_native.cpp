#include <iostream>
#include <vector>
#include <list>
#include <chrono>
#include <sstream>

#include <unistd.h>
#include <string.h>

#include <sys/select.h>
#include <sys/wait.h>

#include "plugin.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"
#include "term_buffer.h"
#include "term_window.h"

#include "PortableThread.h"

#include "term_data_handler_impl.h"
#include "term_data_handler_impl_decl.h"

#include "read_termdata.h"

#include "string_utils.h"

const char * kPathSeparator =
#if defined _WIN32 || defined __CYGWIN__
    "\\";
#else
    "/";
#endif

static
bool load_cap_str(const std::string & termcap_dir,
                  const std::string & term_name,
                  std::string & term_entry) {
    std::string file_path = termcap_dir;
    file_path.append(kPathSeparator).append(term_name).append(".dat");

    return get_entry(file_path, term_name, term_entry);
}

void TermDataHandlerImpl::LoadNativeDataHandler() {
    std::string termcap_dir = GetPluginContext()->GetAppConfig()->GetEntry("/term/termcap_dir", "data");
    std::string term_name = GetPluginContext()->GetAppConfig()->GetEntry("/term/term_name", "xterm-256color");

    std::stringstream ss;

    std::string term_entry;
    if (load_cap_str(termcap_dir, "generic-cap", term_entry))
        ss << term_entry;

    if (!load_cap_str(termcap_dir, term_name, term_entry)) {
        if (load_cap_str(termcap_dir, "xterm-256color", term_entry)) {
            ss << term_entry;
        }
    } else {
        ss << term_entry;
    }

    m_Cap = parse_cap(ss.str());
    m_State = m_Cap->control_data_start_state;
    m_ParseContext = std::make_shared<ControlDataParserContext>();
}

void TermDataHandlerImpl::ProcessSingleCharNative(const char * ch) {
    if (!ch) {
        if (!m_State)
            return;

        HandleCap(false, 0);

        return;
    }

    auto next_state = m_State->handle(m_ParseContext, *ch);
    CapNameMapValue cap_tuple;

    if (!next_state || m_State->get_cap(m_ParseContext->params, cap_tuple)) {
        HandleCap(true, *ch);

        if (next_state) next_state->reset();

        next_state = m_State->handle(m_ParseContext, *ch);

        if (next_state) {
            m_State = next_state;
            if (*ch == 0x1B) {
                m_ControlData.push_back('\\');
                m_ControlData.push_back('E');
            } else {
                m_ControlData.push_back(*ch);
            }
        } else {
            output_char(m_DataContext, *ch, false);
        }

        return;
    }

    m_State = next_state;

    if (*ch == 0x1B) {
        m_ControlData.push_back('\\');
        m_ControlData.push_back('E');
    } else {
        m_ControlData.push_back(*ch);
    }
}

void TermDataHandlerImpl::HandleCap(bool check_unknown, char c) {
    CapNameMapValue cap_tuple;

    auto result = m_State->get_cap(m_ParseContext->params, cap_tuple);

    if (result) {
        bool inc_param = std::get<1>(cap_tuple);

        if (inc_param) {
            for(auto & p : m_ParseContext->params) {
                if (p.has_int_value && p.int_value > 0)
                    p.int_value--;
            }
        }

        handle_cap(m_DataContext,
                   std::get<0>(cap_tuple),
                   m_ParseContext->params);

        if (m_CapStateStack.size() > 0) {
            std::tie(m_State,
                     m_ParseContext->params,
                     m_ControlData) = m_CapStateStack.front();
            m_CapStateStack.pop_front();
        } else {
            m_State = m_Cap->control_data_start_state;
            m_ParseContext->params.clear();
            m_ControlData.clear();
        }
    } else if (check_unknown && m_ControlData.size() > 0) {
        if (!m_Cap->control_data_start_state->handle(m_ParseContext, c)) {
            std::cerr << "unknown control data:" << join(m_ControlData, "") << std::endl;
        }
        m_CapStateStack.push_front(std::make_tuple(m_State,
                                                   m_ParseContext->params,
                                                   m_ControlData));

        m_State = m_Cap->control_data_start_state;
        m_ParseContext->params.clear();
        m_ControlData.clear();
    }
}
