#pragma once

#include "plugin_base.h"
#include "term_window.h"
#include "term_cell.h"

#include <GLFW/glfw3.h>
#include "freetype_gl.h"

#include <bitset>
#include <string>

class DefaultTermWindow :
        public virtual PluginBase
        , public virtual TermWindow {
public:
    DefaultTermWindow(freetype_gl_context_ptr context) :
        PluginBase("term_gl_window", "opengl terminal window plugin", 1)
        , m_MainDlg {nullptr}
        , m_FreeTypeGLContext {context}{
    }

    virtual ~DefaultTermWindow() {
        if (m_MainDlg)
            glfwDestroyWindow(m_MainDlg);
    }

public:
    void Refresh() override;

    void Show() override;

    void Close() override;

    void SetWindowTitle(const std::string & title) override;

    uint32_t GetColorByIndex(uint32_t index) override;

    std::string GetSelectionData() override;

    void SetSelectionData(const std::string & sel_data) override;

    void OnSize(int width, int height);
    bool ShouldClose();
    void OnDraw();

private:
    GLFWwindow * m_MainDlg;
    freetype_gl_context_ptr m_FreeTypeGLContext;
    ftgl::vec4 m_ColorTable[TermCell::ColorIndexCount];

    void DrawContent(std::wstring & content,
                     std::bitset<16> & buffer_mode,
                     uint16_t & last_fore_color,
                     uint16_t & last_back_color,
                     uint16_t & last_mode,
                     uint16_t fore_color,
                     uint16_t back_color,
                     uint16_t mode,
                     float & last_x,
                     float & last_y);
};
