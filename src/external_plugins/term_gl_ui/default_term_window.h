#pragma once

#include "plugin_base.h"
#include "term_window.h"
#include <GLFW/glfw3.h>
#include "freetype_gl.h"

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
private:
    GLFWwindow * m_MainDlg;
    freetype_gl_context_ptr m_FreeTypeGLContext;
};
