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
InputHandler::KeyCodeEnum wxKeyToInputHandlerKey(wxKeyCode uc) {
    if (uc >= '0' && uc <= '9')
        return (InputHandler::KeyCodeEnum)uc;

    if (uc >= 'A' && uc <= 'Z')
        return (InputHandler::KeyCodeEnum)uc;

    switch((int)uc) {
    case 39 : return InputHandler::KEY_APOSTROPHE;
    case 44 : return InputHandler::KEY_COMMA;
    case 45 : return InputHandler::KEY_MINUS;
    case 46 : return InputHandler::KEY_PERIOD;
    case 47 : return InputHandler::KEY_SLASH;
    case 59 : return InputHandler::KEY_SEMICOLON;
    case 61 : return InputHandler::KEY_EQUAL;
    case 91 : return InputHandler::KEY_LEFT_BRACKET;
    case 92 : return InputHandler::KEY_BACKSLASH;
    case 93 : return InputHandler::KEY_RIGHT_BRACKET;
    case 96 : return InputHandler::KEY_GRAVE_ACCENT;
    default:
        break;
    }

    switch(uc) {
    case WXK_SPACE : return InputHandler::KEY_SPACE;
    case WXK_ESCAPE : return InputHandler::KEY_ESCAPE;
    case WXK_RETURN : return InputHandler::KEY_ENTER;
    case WXK_TAB : return InputHandler::KEY_TAB;
    case WXK_BACK : return InputHandler::KEY_BACKSPACE;
    case WXK_INSERT : return InputHandler::KEY_INSERT;
    case WXK_DELETE : return InputHandler::KEY_DELETE;
    case WXK_RIGHT : return InputHandler::KEY_RIGHT;
    case WXK_LEFT : return InputHandler::KEY_LEFT;
    case WXK_DOWN : return InputHandler::KEY_DOWN;
    case WXK_UP : return InputHandler::KEY_UP;
    case WXK_PAGEUP : return InputHandler::KEY_PAGE_UP;
    case WXK_PAGEDOWN : return InputHandler::KEY_PAGE_DOWN;
    case WXK_HOME : return InputHandler::KEY_HOME;
    case WXK_END : return InputHandler::KEY_END;
    case WXK_CAPITAL : return InputHandler::KEY_CAPS_LOCK;
    case WXK_SCROLL : return InputHandler::KEY_SCROLL_LOCK;
    case WXK_NUMLOCK : return InputHandler::KEY_NUM_LOCK;
    case WXK_PRINT : return InputHandler::KEY_PRINT_SCREEN;
    case WXK_PAUSE : return InputHandler::KEY_PAUSE;
    case WXK_F1 : return InputHandler::KEY_F1;
    case WXK_F2 : return InputHandler::KEY_F2;
    case WXK_F3 : return InputHandler::KEY_F3;
    case WXK_F4 : return InputHandler::KEY_F4;
    case WXK_F5 : return InputHandler::KEY_F5;
    case WXK_F6 : return InputHandler::KEY_F6;
    case WXK_F7 : return InputHandler::KEY_F7;
    case WXK_F8 : return InputHandler::KEY_F8;
    case WXK_F9 : return InputHandler::KEY_F9;
    case WXK_F10 : return InputHandler::KEY_F10;
    case WXK_F11 : return InputHandler::KEY_F11;
    case WXK_F12 : return InputHandler::KEY_F12;
    case WXK_F13 : return InputHandler::KEY_F13;
    case WXK_F14 : return InputHandler::KEY_F14;
    case WXK_F15 : return InputHandler::KEY_F15;
    case WXK_F16 : return InputHandler::KEY_F16;
    case WXK_F17 : return InputHandler::KEY_F17;
    case WXK_F18 : return InputHandler::KEY_F18;
    case WXK_F19 : return InputHandler::KEY_F19;
    case WXK_F20 : return InputHandler::KEY_F20;
    case WXK_F21 : return InputHandler::KEY_F21;
    case WXK_F22 : return InputHandler::KEY_F22;
    case WXK_F23 : return InputHandler::KEY_F23;
    case WXK_F24 : return InputHandler::KEY_F24;
    case WXK_NUMPAD0 : return InputHandler::KEY_KP_0;
    case WXK_NUMPAD1 : return InputHandler::KEY_KP_1;
    case WXK_NUMPAD2 : return InputHandler::KEY_KP_2;
    case WXK_NUMPAD3 : return InputHandler::KEY_KP_3;
    case WXK_NUMPAD4 : return InputHandler::KEY_KP_4;
    case WXK_NUMPAD5 : return InputHandler::KEY_KP_5;
    case WXK_NUMPAD6 : return InputHandler::KEY_KP_6;
    case WXK_NUMPAD7 : return InputHandler::KEY_KP_7;
    case WXK_NUMPAD8 : return InputHandler::KEY_KP_8;
    case WXK_NUMPAD9 : return InputHandler::KEY_KP_9;
    case WXK_NUMPAD_DECIMAL : return InputHandler::KEY_KP_DECIMAL;
    case WXK_NUMPAD_DIVIDE : return InputHandler::KEY_KP_DIVIDE;
    case WXK_NUMPAD_MULTIPLY : return InputHandler::KEY_KP_MULTIPLY;
    case WXK_NUMPAD_SUBTRACT : return InputHandler::KEY_KP_SUBTRACT;
    case WXK_NUMPAD_ADD : return InputHandler::KEY_KP_ADD;
    case WXK_NUMPAD_ENTER : return InputHandler::KEY_KP_ENTER;
    case WXK_NUMPAD_EQUAL : return InputHandler::KEY_KP_EQUAL;
    case WXK_SHIFT : return InputHandler::KEY_LEFT_SHIFT;
    case WXK_RAW_CONTROL : return InputHandler::KEY_LEFT_CONTROL;
    case WXK_ALT : return InputHandler::KEY_LEFT_ALT;
    case WXK_COMMAND : return InputHandler::KEY_LEFT_SUPER;
    case WXK_MENU : return InputHandler::KEY_MENU;
    default:
        return InputHandler::KEY_UNKNOWN;
    }
}

void DrawPane::OnKeyDown(wxKeyEvent& event)
{
    InputHandler::KeyCodeEnum key = wxKeyToInputHandlerKey((wxKeyCode)event.GetKeyCode());

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(m_TermWindow->GetPluginContext());

    if (!context)
        return;

    InputHandlerPtr input_handler = context->GetInputHandler();

    int mods = 0;

    if (event.ShiftDown()) mods |= InputHandler::ModifierEnum::MOD_SHIFT;
    if (event.AltDown()) mods |= InputHandler::ModifierEnum::MOD_ALT;
    if (event.RawControlDown()) mods |= InputHandler::ModifierEnum::MOD_CONTROL;

    if (!input_handler->ProcessKey((InputHandler::KeyCodeEnum)key,
                                  (InputHandler::ModifierEnum)mods,
                                   true)) {
        event.Skip();
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

    InputHandlerPtr input_handler = context->GetInputHandler();

    input_handler->ProcessCharInput((int32_t)uc, (InputHandler::ModifierEnum)0);
}
