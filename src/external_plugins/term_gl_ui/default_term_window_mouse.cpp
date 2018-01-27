#include <pybind11/embed.h>

#include "default_term_window.h"

#include "term_buffer.h"
#include "term_context.h"
#include "term_network.h"
#include "color_theme.h"

#include "char_width.h"

#include <iostream>


static
void send_data(TermNetworkPtr &network, const std::vector<unsigned char> & data) {
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
};

static
void save_number(std::vector<unsigned char> & data, uint32_t v) {
    char buf[255] {0};

    sprintf(buf, "%u", v);

    for(size_t i = 0; i < strlen(buf); i++)
        data.push_back(buf[i]);
}

void DefaultTermWindow::OnMouseWheel(double xoffset, double yoffset)
{
    (void)xoffset;
    (void)yoffset;
    std::cout << "wheel mouse, x:" << xoffset << ", y:" << yoffset << std::endl;

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    TermNetworkPtr network = context->GetTermNetwork();
    TermBufferPtr buffer = context->GetTermBuffer();

    std::vector<unsigned char> data;
    data.push_back('\x1B');
    data.push_back('[');
    data.push_back('<');

    auto rows = buffer->GetRows();

    auto row = buffer->GetRow(), old_row = row;
    auto col = buffer->GetCol();

    if (yoffset > 0) {
        save_number(data, 64);
        if (row > yoffset) {
            row -= yoffset;
        } else {
            row = 0;
        }
    } else {
        save_number(data, 65);
        if (row - yoffset < rows) {
            row -= yoffset;
        } else {
            row = rows - 1;
        }
    }

    data.push_back(';');
    save_number(data, col + 1);
    data.push_back(';');
    save_number(data, row + 1);
    data.push_back('M');

    if (old_row == row)
        return;
    send_data(network, data);
}

void DefaultTermWindow::OnMouseButton(int button, int action, int mods, double xpos, double ypos)
{
    (void)button;
    (void)action;
    (void)mods;
    (void)xpos;
    (void)ypos;
}

void DefaultTermWindow::OnMouseMove(double xpos, double ypos)
{
    (void)xpos;
    (void)ypos;
}
