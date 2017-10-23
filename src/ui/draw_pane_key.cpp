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
        printf("key code:%d\n", event.GetKeyCode());

        return ;
    }

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    TermNetworkPtr network = context->GetTermNetwork();

   try
    {
        pybind11::gil_scoped_acquire acquire;
        char ch[2] = {c, 0};
        network->Send(ch, 1);
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
