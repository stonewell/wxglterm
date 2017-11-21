#include "term_buffer.h"
#include "term_window.h"
#include "cap_manager.h"

void set_window_properties(term_data_context_s & term_context,
                           int mode,
                           const std::string & data)
{
    switch(mode)
    {
    case 0:
    case 3:
        term_context.term_window->SetWindowTitle(data);
        break;
    default:
        break;
    }
}
