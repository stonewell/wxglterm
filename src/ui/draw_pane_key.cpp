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
    wxChar uc = event.GetUnicodeKey();

    char c= uc & 0xFF;

    if (c >= 'A' && c <= 'Z' && !event.ShiftDown()) {
        c = c - 'A' + 'a';
    }

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    TermNetworkPtr network = context->GetTermNetwork();

    std::vector<unsigned char> data;

    if (uc != WXK_NONE && (event.GetModifiers() & wxMOD_ALT)){
        data.push_back('\x1B');
    }

    if (m_AppDebug) {
        std::cout << "on key down:" << (int)c << ","
                  << event.AltDown() << ","
                  << event.RawControlDown()
                  << ","
                  << uc
                  << ","
                  << event.GetKeyCode()
                  << std::endl;
    }

    if (uc != WXK_NONE && (event.GetModifiers() & wxMOD_RAW_CONTROL)) {
        if (uc >= 'a' && uc <= 'z')
            data.push_back((char)(uc - 'a' + 1));
        if (uc >= 'A' && uc <= 'Z')
            data.push_back((char)(uc - 'A' + 1));
        else if (uc>= '[' && uc <= ']')
            data.push_back((char)(uc - '[' + 27));
        else if (uc == '6')
            data.push_back((char)('^' - '[' + 27));
        else if (uc == '-')
            data.push_back((char)('_' - '[' + 27));
    }

    if (uc == WXK_RETURN && !event.HasModifiers())
    {
        data.push_back((char)13);
    }

    if (data.size() == 0) {
        event.Skip();
        return;
    }

    //add char when there only ALT pressed
    if (data[0] == '\x1B')
        data.push_back(c);

    try
    {
        pybind11::gil_scoped_acquire acquire;

        network->Send(data, data.size());
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

void DrawPane::OnKeyUp(wxKeyEvent& event)
{
    (void)event;
}

void DrawPane::OnChar(wxKeyEvent& event)
{
    wxChar uc = event.GetUnicodeKey();

    if (uc == WXK_NONE)
    {
        return;
    }

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    TermNetworkPtr network = context->GetTermNetwork();

    std::vector<unsigned char> data;

    data.push_back(uc);

    try
    {
        pybind11::gil_scoped_acquire acquire;

        network->Send(data, data.size());
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
