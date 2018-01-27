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

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    TermNetworkPtr network = context->GetTermNetwork();
    TermBufferPtr buffer = context->GetTermBuffer();

    std::vector<unsigned char> data;
    data.push_back('\x1B');
    data.push_back('[');
    data.push_back('<');

    int height = 0;
    glfwGetFramebufferSize(m_MainDlg, NULL, &height);

    int w_height;
    glfwGetWindowSize(m_MainDlg, NULL, &w_height);

    double ypos;
    glfwGetCursorPos(m_MainDlg, NULL, &ypos);

    ypos = (ypos / w_height) * height;

    auto row = (ypos > PADDING ? ypos - PADDING : 0) / m_FreeTypeGLContext->line_height;
    auto col = buffer->GetCol();

    if (yoffset > 0) {
        save_number(data, 64);
    } else {
        save_number(data, 65);
    }

    data.push_back(';');
    save_number(data, col + 1);
    data.push_back(';');
    save_number(data, row + 1);
    data.push_back('M');

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
