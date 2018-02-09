#include <pybind11/embed.h>

#include <iostream>
#include <unistd.h>
#include <vector>

#include <string.h>

#include "key_code_map.h"

#include "plugin_manager.h"
#include "plugin.h"
#include "term_network.h"
#include "term_data_handler.h"
#include "term_context.h"
#include "term_window.h"
#include "input.h"
#include "plugin_base.h"

#include "app_config_impl.h"

#include <locale>
#include <codecvt>
#include <functional>
#include "base64.h"
#include "input_handler_plugin.h"

#include "network_utils.h"

static
std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcharconv;

bool DefaultInputHandler::ProcessKey(InputHandler::KeyCodeEnum key, InputHandler::ModifierEnum mods, bool down) {
    (void)down;
    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return false;

    TermNetworkPtr network = context->GetTermNetwork();
    TermWindowPtr window = context->GetTermWindow();

    std::vector<unsigned char> data;

    bool paste_data =
            (mods == InputHandler::MOD_SHIFT && key == InputHandler::KEY_INSERT)
#ifdef __APPLE__
            || (mods == InputHandler::MOD_SUPER && key == InputHandler::KEY_V)
#endif
            ;
    if (paste_data) {
        //paste from clipboard
        auto clipboard_data = window->GetSelectionData();

        if (clipboard_data.size() > 0) {
            std::string decoded {};
            if (Base64::Decode(clipboard_data, &decoded)) {
                std::copy(decoded.begin(),
                          decoded.end(),
                          std::back_inserter(data));

                send_data(network, data);
            }
        }

        return true;
    }

    if (mods & InputHandler::MOD_ALT){
        data.push_back('\x1B');
    }

    if (key != InputHandler::KEY_UNKNOWN && (mods & InputHandler::MOD_CONTROL)) {
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

    //add char when there only ALT pressed
    if (data.size() == 1 && data[0] == '\x1B' && key >= 0 && key <0x80) {
        if (!(mods & InputHandler::MOD_SHIFT) && (key >= 'A' && key <= 'Z')) {
            key = (InputHandler::KeyCodeEnum)(key - 'A' + 'a');
        }
        data.push_back((char)key);
    }

    if (data.size() == 0)
        return false;

    send_data(network, data);
    return true;
}

bool DefaultInputHandler::ProcessCharInput(int32_t codepoint, InputHandler::ModifierEnum modifier) {
    (void)codepoint;
    (void)modifier;
    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return false;

    TermNetworkPtr network = context->GetTermNetwork();

    std::vector<unsigned char> data;

    std::string bytes = wcharconv.to_bytes((wchar_t)codepoint);

    for(std::string::size_type i=0; i < bytes.length(); i++)
        data.push_back(bytes[i]);

    send_data(network, data);
    return true;
}
