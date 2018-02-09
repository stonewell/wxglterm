#include <pybind11/embed.h>

#include "default_term_window.h"

#include "term_buffer.h"
#include "term_context.h"
#include "term_network.h"
#include "color_theme.h"
#include "input.h"

void DefaultTermWindow::GetMousePos(uint32_t & row, uint32_t & col)
{
    int height = 0, width = 0;
    glfwGetFramebufferSize(m_MainDlg, &width, &height);

    int w_height, w_width;
    glfwGetWindowSize(m_MainDlg, &w_width, &w_height);

    double ypos, xpos;
    glfwGetCursorPos(m_MainDlg, &xpos, &ypos);

    ypos = (ypos / w_height) * height;
    xpos = (xpos / w_width) * width;

    row = (ypos > PADDING ? ypos - PADDING : 0) / m_FreeTypeGLContext->line_height;
    col = (xpos > PADDING ? xpos - PADDING : 0) / m_FreeTypeGLContext->col_width;
}

void DefaultTermWindow::OnMouseWheel(double xoffset, double yoffset)
{
    if (!m_EnableMouseTrack) return;

    (void)xoffset;
    (void)yoffset;

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    InputHandlerPtr input_handler = context->GetInputHandler();

    uint32_t row = 0, col = 0;
    GetMousePos(row, col);

    input_handler->ProcessMouseButton((InputHandler::MouseButtonEnum)(yoffset > 0 ? 4 : 5),
                                      col,
                                      row,
                                      (InputHandler::ModifierEnum)0,
                                      true);
}

void DefaultTermWindow::OnMouseButton(int button, int action, int mods, double xpos, double ypos)
{
    if (!m_EnableMouseTrack) return;

    (void)button;
    (void)action;
    (void)mods;
    (void)xpos;
    (void)ypos;

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    InputHandlerPtr input_handler = context->GetInputHandler();

    if (action == GLFW_PRESS)
        m_SavedMouseButton = button;
    else
        m_SavedMouseButton = -1;

    uint32_t row = 0, col = 0;
    GetMousePos(row, col);

    input_handler->ProcessMouseButton((InputHandler::MouseButtonEnum)button,
                                      col,
                                      row,
                                      (InputHandler::ModifierEnum)mods,
                                      action == GLFW_PRESS);
}

void DefaultTermWindow::OnMouseMove(double xpos, double ypos)
{
    if (!m_EnableMouseTrack) return;

    (void)xpos;
    (void)ypos;
    if (m_SavedMouseButton < 0)
        return;

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    uint32_t row = 0, col = 0;
    GetMousePos(row, col);

    InputHandlerPtr input_handler = context->GetInputHandler();

    input_handler->ProcessMouseMove((InputHandler::MouseButtonEnum)m_SavedMouseButton,
                                    col,
                                    row,
                                    (InputHandler::ModifierEnum)0);
}
