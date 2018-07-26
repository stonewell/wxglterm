#include <pybind11/embed.h>

#include "default_term_window.h"

#include "term_buffer.h"
#include "term_context.h"
#include "term_network.h"
#include "color_theme.h"
#include "input.h"

#include "char_width.h"

#include <iostream>
#include <iterator>
#include <functional>
#include <locale>
#include <codecvt>

static
bool IgnoreKey(int key, int mods) {
    (void)mods;
    //ignore all single ctrl keys
    switch((InputHandler::KeyCodeEnum)key)
    {
    case InputHandler::KEY_LEFT_SHIFT:
    case InputHandler::KEY_LEFT_CONTROL:
    case InputHandler::KEY_LEFT_ALT:
    case InputHandler::KEY_LEFT_SUPER:
    case InputHandler::KEY_RIGHT_SHIFT:
    case InputHandler::KEY_RIGHT_CONTROL:
    case InputHandler::KEY_RIGHT_ALT:
    case InputHandler::KEY_RIGHT_SUPER:
    case InputHandler::KEY_MENU:
        return true;
    default:
        return false;
    }
}

void DefaultTermWindow::OnKeyDown(int key, int scancode, int mods, bool repeat) {
    (void)scancode;
    m_ProcessedKey = m_ProcessedMod = 0;

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    InputHandlerPtr input_handler = context->GetInputHandler();

    if (IgnoreKey(key, mods))
        return;

    if (!input_handler->ProcessKey((InputHandler::KeyCodeEnum)key,
                                  (InputHandler::ModifierEnum)mods,
                                   true)) {
        if (repeat) {
            if (!(mods & GLFW_MOD_SHIFT) && (key >= 'A' && key <= 'Z')) {
                key = key - 'A' + 'a';
            }
            OnChar(key, mods);
            m_ProcessedKey = key;
            m_ProcessedMod = mods;
        }
    }
}

static
unsigned int translate_shift(unsigned int codepoint) {
    static bool init = false;
    static char code[0x80] = {0};

    if (codepoint >= 0x80)
        return codepoint;

    if (!init) {
        for(int i=0;i<0x80;i++) {
            code[i] = i;
        }

        for(int i='a';i<='z';i++) {
            code[i] = 'A' + i - 'a';
        }
        code[(int)'`'] = '~';
        code[(int)'1'] = '!';
        code[(int)'2'] = '@';
        code[(int)'3'] = '#';
        code[(int)'4'] = '$';
        code[(int)'5'] = '%';
        code[(int)'6'] = '^';
        code[(int)'7'] = '&';
        code[(int)'8'] = '*';
        code[(int)'9'] = '(';
        code[(int)'0'] = ')';
        code[(int)'-'] = '_';
        code[(int)'='] = '+';
        code[(int)'['] = '{';
        code[(int)']'] = '}';
        code[(int)'\\'] = '|';
        code[(int)';'] = ':';
        code[(int)'\''] = '"';
        code[(int)','] = '<';
        code[(int)'.'] = '>';
        code[(int)'/'] = '?';

        init = true;
    }

    return code[codepoint];
}

void DefaultTermWindow::OnChar(unsigned int codepoint, int mods) {
    (void)mods;

    if (codepoint == m_ProcessedKey && mods == m_ProcessedMod) {
        return;
    }

    if (IgnoreKey(codepoint, mods))
        return;

    //do not handle other modifiers in char callback except shift only
    if (mods != 0) {
        if (mods == GLFW_MOD_SHIFT) {
            codepoint = translate_shift(codepoint);
        } else {
            return;
        }
    }

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;
    InputHandlerPtr input_handler = context->GetInputHandler();

    input_handler->ProcessCharInput(codepoint, (InputHandler::ModifierEnum)mods);
}
