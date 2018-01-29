#include <pybind11/embed.h>

#include "default_term_window.h"

#include "term_buffer.h"
#include "term_context.h"
#include "term_network.h"
#include "color_theme.h"

#include "char_width.h"
#include "string_utils.h"

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

void DefaultTermWindow::SendMouseEvent(int button, bool press, bool motion) {
    if (!m_EnableMouseTrack) return;

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    TermNetworkPtr network = context->GetTermNetwork();
    TermBufferPtr buffer = context->GetTermBuffer();

    std::vector<unsigned char> data;
    data.push_back('\x1B');
    data.push_back('[');
    data.push_back('<');

    int height = 0, width = 0;
    glfwGetFramebufferSize(m_MainDlg, &width, &height);

    int w_height, w_width;
    glfwGetWindowSize(m_MainDlg, &w_width, &w_height);

    double ypos, xpos;
    glfwGetCursorPos(m_MainDlg, &xpos, &ypos);

    ypos = (ypos / w_height) * height;
    xpos = (xpos / w_width) * width;

    auto row = (ypos > PADDING ? ypos - PADDING : 0) / m_FreeTypeGLContext->line_height;
    auto col = (xpos > PADDING ? xpos - PADDING : 0) / m_FreeTypeGLContext->col_width;

    save_number(data, mouse_btn_code(button, motion));

    data.push_back(';');
    save_number(data, col + 1);
    data.push_back(';');
    save_number(data, row + 1);
    data.push_back(press ? 'M' : 'm');

    send_data(network, data);
}

void DefaultTermWindow::OnMouseWheel(double xoffset, double yoffset)
{
    (void)xoffset;
    (void)yoffset;

    SendMouseEvent(yoffset > 0 ? 4 : 5,
                   true,
                   false);
}

void DefaultTermWindow::OnMouseButton(int button, int action, int mods, double xpos, double ypos)
{
    (void)button;
    (void)action;
    (void)mods;
    (void)xpos;
    (void)ypos;

    if (action == GLFW_PRESS)
        m_SavedMouseButton = button;
    else
        m_SavedMouseButton = -1;
    SendMouseEvent(button, action == GLFW_PRESS, false);
}

void DefaultTermWindow::OnMouseMove(double xpos, double ypos)
{
    (void)xpos;
    (void)ypos;
    if (m_SavedMouseButton < 0)
        return;

    SendMouseEvent(m_SavedMouseButton, GLFW_PRESS, true);
}
