#include <pybind11/embed.h>

#include "default_term_window.h"

#include "term_buffer.h"
#include "term_context.h"
#include "term_network.h"
#include "color_theme.h"

#include "char_width.h"

#include "shader.h"

#include <iostream>
#include <iterator>
#include <functional>
#include <locale>
#include <codecvt>

static
std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcharconv;

void DefaultTermWindow::OnKeyDown(int key, int scancode, int mods) {
    (void)scancode;

    std::cout << "key:" << key << ", scancode:" << scancode << ", mods:" << mods << std::endl;
    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    char c = key & 0xFF;

    if (c >= 'A' && c <= 'Z' && (mods & GLFW_MOD_SHIFT)) {
        c = c - 'A' + 'a';
    }

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
        // if (wxTheClipboard->Open()) {
        //     if (wxTheClipboard->IsSupported( wxDF_TEXT ) ||
        //         wxTheClipboard->IsSupported( wxDF_UNICODETEXT) ||
        //         wxTheClipboard->IsSupported( wxDF_OEMTEXT)) {
        //         wxTextDataObject txt_data;
        //         wxTheClipboard->GetData( txt_data );

        //         if (txt_data.GetTextLength() > 0) {
        //             wxString s = txt_data.GetText();
        //             const auto & s_buf = s.utf8_str();
        //             const char * s_begin = s_buf;
        //             const char * s_end = s_begin + s_buf.length();

        //             std::copy(s_begin,
        //                       s_end,
        //                       std::back_inserter(data));

        //             send_data(data);
        //         }
        //     }
        //     wxTheClipboard->Close();
        // }

        return;
    }

    if (key == GLFW_KEY_UNKNOWN && (mods & GLFW_MOD_ALT)){
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

    if (key == GLFW_KEY_ENTER && (mods == 0))
    {
        data.push_back((char)13);
    }

    if (data.size() == 0) {
        return;
    }

    //add char when there only ALT pressed
    if (data.size() == 1 && data[0] == '\x1B')
        data.push_back(c);

    send_data(data);
}

void DefaultTermWindow::OnChar(unsigned int codepoint, int mods) {
    (void)mods;

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
