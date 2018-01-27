#include <pybind11/embed.h>

#include "default_term_window.h"

#include "term_buffer.h"
#include "term_context.h"
#include "term_network.h"
#include "color_theme.h"

#include "char_width.h"

#include "shader.h"

#include "key_code_map.h"

#include <iostream>
#include <iterator>
#include <functional>
#include <locale>
#include <codecvt>

static
std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcharconv;

void DefaultTermWindow::OnKeyDown(int key, int scancode, int mods, bool repeat) {
    (void)scancode;
    m_ProcessedKey = m_ProcessedMod = 0;

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    TermNetworkPtr network = context->GetTermNetwork();

    std::function<void(const std::vector<unsigned char> & data)> send_data {
        [network](const std::vector<unsigned char> & data) {
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
    };

    std::vector<unsigned char> data;

    bool paste_data =
            (mods == GLFW_MOD_SHIFT && key == GLFW_KEY_INSERT)
#ifdef __APPLE__
            || (mods == GLFW_MOD_SUPER && key == GLFW_KEY_V)
#endif
            ;
    if (paste_data) {
        //paste from clipboard
        auto clipboard_data = glfwGetClipboardString(m_MainDlg);

        if (clipboard_data) {
            const char * s_begin = clipboard_data;
            const char * s_end = clipboard_data + strlen(clipboard_data);

            std::copy(s_begin,
                      s_end,
                      std::back_inserter(data));

            send_data(data);
        }

        return;
    }

    if (mods & GLFW_MOD_ALT){
        data.push_back('\x1B');
    }

    if (key != GLFW_KEY_UNKNOWN && (mods & GLFW_MOD_CONTROL)) {
        if (key >= 'a' && key <= 'z')
            data.push_back((char)(key - 'a' + 1));
        if (key >= 'A' && key <= 'Z')
            data.push_back((char)(key - 'A' + 1));
        else if (key>= '[' && key <= ']')
            data.push_back((char)(key - '[' + 27));
        else if (key == '6')
            data.push_back((char)('^' - '[' + 27));
        else if (key == '-')
            data.push_back((char)('_' - '[' + 27));
    }

    if (mods == 0) {
        const char * c = get_mapped_key_code(key);

        if (c) {
            for(size_t i=0;i<strlen(c);i++)
                data.push_back(c[i]);
        }
    }

    if (data.size() == 0) {
        if (repeat) {
            if (!(mods & GLFW_MOD_SHIFT) && (key >= 'A' && key <= 'Z')) {
                key = key - 'A' + 'a';
            }
            OnChar(key, mods);
            m_ProcessedKey = key;
            m_ProcessedMod = mods;
        }
        return;
    }

    //add char when there only ALT pressed
    if (data.size() == 1 && data[0] == '\x1B' && key >= 0 && key <0x80) {
        if (!(mods & GLFW_MOD_SHIFT) && (key >= 'A' && key <= 'Z')) {
            key = key - 'A' + 'a';
        }
        data.push_back((char)key);
    }

    send_data(data);
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

    if (codepoint == m_ProcessedKey && mods == m_ProcessedMod)
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

    TermNetworkPtr network = context->GetTermNetwork();

    std::vector<unsigned char> data;

    std::string bytes = wcharconv.to_bytes((wchar_t)codepoint);

    for(std::string::size_type i=0; i < bytes.length(); i++)
        data.push_back(bytes[i]);

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
