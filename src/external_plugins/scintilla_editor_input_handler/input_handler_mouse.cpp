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
void save_number(std::vector<unsigned char> & data, uint32_t v) {
    char buf[255] {0};

    sprintf(buf, "%u", v);

    for(size_t i = 0; i < strlen(buf); i++)
        data.push_back(buf[i]);
}

static int
mouse_btn_code(int button, bool motion)
{
    int result = 0;

    if (motion)
        result += 32;

    if (button < 0 || button > 5) {
        result += 3;
    } else {
        if (button > 3)
            result += (64 - 4);
        result += button;
    }
    return result;
}

void DefaultInputHandler::SendMouseEvent(int button, bool press, bool motion, uint32_t col, uint32_t row) {
    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    TermNetworkPtr network = context->GetTermNetwork();
    TermBufferPtr buffer = context->GetTermBuffer();

    std::vector<unsigned char> data;
    data.push_back('\x1B');
    data.push_back('[');
    data.push_back('<');

    save_number(data, mouse_btn_code(button, motion));

    data.push_back(';');
    save_number(data, col + 1);
    data.push_back(';');
    save_number(data, row + 1);
    data.push_back(press ? 'M' : 'm');

    send_data(network, data);
}

bool DefaultInputHandler::ProcessMouseButton(InputHandler::MouseButtonEnum btn, uint32_t col, uint32_t row, InputHandler::ModifierEnum modifier, bool down) {
    (void)btn;
    (void)col;
    (void)row;
    (void)modifier;
    (void)down;
    SendMouseEvent(btn, down, false, col, row);
    return true;
}

bool DefaultInputHandler::ProcessMouseMove(InputHandler::MouseButtonEnum btn, uint32_t col, uint32_t row, InputHandler::ModifierEnum modifier) {
    (void)btn;
    (void)col;
    (void)row;
    (void)modifier;
    SendMouseEvent(btn, true, true, col, row);
    return false;
}
