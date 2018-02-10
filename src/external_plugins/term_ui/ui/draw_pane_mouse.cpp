#include <pybind11/embed.h>

#include <iostream>
#include <iterator>
#include <functional>
#include <locale>
#include <codecvt>

#include "term_window.h"
#include "draw_pane.h"

#include <wx/dcbuffer.h>
#include <wx/clipbrd.h>

#include "term_context.h"
#include "term_buffer.h"
#include "term_line.h"
#include "term_cell.h"
#include "term_network.h"
#include "color_theme.h"
#include "input.h"

static
int wxMouseButtonToInputHandlerMouseButton(int btn) {
    switch(btn) {
    case wxMOUSE_BTN_LEFT:
        return InputHandler::MOUSE_BUTTON_LEFT;
    case wxMOUSE_BTN_RIGHT:
        return InputHandler::MOUSE_BUTTON_RIGHT;
    case wxMOUSE_BTN_MIDDLE:
        return InputHandler::MOUSE_BUTTON_MIDDLE;
    default:
        return -1;
    }
}

extern
wxCoord PADDING;

void DrawPane::GetMousePos(wxMouseEvent & event, uint32_t & row, uint32_t & col)
{
    int x = 0, y = 0;
    event.GetPosition(&x, &y);

    row = (y > PADDING ? y - PADDING : 0) / m_LineHeight;
    col = (x > PADDING ? x - PADDING : 0) / m_CellWidth;
}

void DrawPane::OnMouseEvent(wxMouseEvent& event) {
    if (!m_EnableMouseTrack) return;

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    InputHandlerPtr input_handler = context->GetInputHandler();

    uint32_t row = 0, col = 0;
    GetMousePos(event, row, col);

    wxEventType et = event.GetEventType();

    int button = wxMouseButtonToInputHandlerMouseButton(event.GetButton());

    if (et == wxEVT_LEFT_DOWN ||
        et == wxEVT_MIDDLE_DOWN ||
        et == wxEVT_RIGHT_DOWN) {
        m_SavedMouseButton = button;
        input_handler->ProcessMouseButton((InputHandler::MouseButtonEnum)button,
                                          col,
                                          row,
                                          (InputHandler::ModifierEnum)0,
                                          true);
    }

    if (et == wxEVT_LEFT_UP ||
        et == wxEVT_MIDDLE_UP ||
        et == wxEVT_RIGHT_UP) {
        m_SavedMouseButton = -1;
        input_handler->ProcessMouseButton((InputHandler::MouseButtonEnum)button,
                                          col,
                                          row,
                                          (InputHandler::ModifierEnum)0,
                                          false);
    }

    if (et == wxEVT_MOTION && m_SavedMouseButton != -1) {
        input_handler->ProcessMouseMove((InputHandler::MouseButtonEnum)m_SavedMouseButton,
                                        col,
                                        row,
                                        (InputHandler::ModifierEnum)0);
    }

    if (et == wxEVT_MOUSEWHEEL) {
        input_handler->ProcessMouseButton((InputHandler::MouseButtonEnum)(event.GetWheelRotation() > 0 ? 4 : 5),
                                          col,
                                          row,
                                          (InputHandler::ModifierEnum)0,
                                          true);
    }
}
