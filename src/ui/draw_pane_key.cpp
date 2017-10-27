#include <pybind11/embed.h>

#include <iostream>

#include "term_window.h"
#include "draw_pane.h"
#include <wx/dcbuffer.h>

#include "term_context.h"
#include "term_buffer.h"
#include "term_line.h"
#include "term_cell.h"
#include "term_network.h"
#include "color_theme.h"

void DrawPane::OnKeyDown(wxKeyEvent& event)
{
    (void)event;
    event.Skip();
}

void DrawPane::OnKeyUp(wxKeyEvent& event)
{
    (void)event;
}

void DrawPane::OnChar(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();

    char c= uc & 0xFF;
    if (uc == WXK_NONE)
    {
        uc = event.GetKeyCode();
    }

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    TermNetworkPtr network = context->GetTermNetwork();

    std::vector<char> data;

    bool char_processed = false;
    if (event.AltDown()){
        data.push_back('\x1B');
    }

    if (event.ControlDown()) {
        char_processed = true;
        if (c >= 'a' && c <= 'z')
            data.push_back((char)(c - 'a' + 1));
        else if (c>= '[' && c <= ']')
            data.push_back((char)(c - '[' + 27));
        else if (c == '6')
            data.push_back((char)('^' - '[' + 27));
        else if (c == '-')
            data.push_back((char)('_' - '[' + 27));
        else
            char_processed = false;

    }

    if (!char_processed)
        data.push_back(c);

    try
    {
        pybind11::gil_scoped_acquire acquire;

        network->Send(&data[0], data.size());
    }
    catch(std::exception & e)
    {
        std::cerr << "!!Error Send:"
                  << std::endl
                  << e.what()
                  << std::endl;
        PyErr_Print();
    }
    catch(...)
    {
        std::cerr << "!!Error Send"
                  << std::endl;
        PyErr_Print();
    }
}
