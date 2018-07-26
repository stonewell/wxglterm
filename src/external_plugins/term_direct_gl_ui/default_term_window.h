#pragma once

#include "plugin_base.h"
#include "term_window.h"
#include "term_cell.h"

#include "freetype_gl.h"
#include "text_buffer.h"
#include "render.h"

#include <GLFW/glfw3.h>
#include "glm/glm.hpp"

#include <bitset>
#include <string>
#include <mutex>

class DefaultTermWindow :
        public virtual PluginBase
        , public virtual TermWindow {
public:
    enum {
        PADDING = 5,
    };

    DefaultTermWindow();

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
    void SetColorByIndex(uint32_t index, uint32_t v) override;

    std::string GetSelectionData() override;

    void SetSelectionData(const std::string & sel_data) override;
    void EnableMouseTrack(bool enable) override;

    void OnSize(int width, int height);
    bool ShouldClose();
    void OnDraw();
    void UpdateWindow();

    void OnKeyDown(int key, int scancode, int mods, bool repeat);
    void OnChar(unsigned int codepoint, int mods);
    void OnMouseWheel(double xoffset, double yoffset);
    void OnMouseButton(int button, int action, int mods, double xpos, double ypos);
    void OnMouseMove(double xpos, double ypos);

    uint32_t GetWidth() override {
        return m_Width;
    }

    uint32_t GetHeight() override {
        return m_Height;
    }

    uint32_t GetLineHeight() override {
        return m_FreeTypeGLContext->line_height;
    }

    uint32_t GetColWidth() override {
        return m_FreeTypeGLContext->col_width;
    }
private:
    GLFWwindow * m_MainDlg;
    freetype_gl_context_ptr m_FreeTypeGLContext;
    ftdgl::text::color_s  m_ColorTable[TermCell::ColorIndexCount];
    ftdgl::text::TextBufferPtr m_TextBuffer;
    std::mutex m_RefreshLock;
    int m_RefreshNow;
    unsigned int m_ProcessedKey;
    int m_ProcessedMod;
    int m_SavedMouseButton;
    bool m_EnableMouseTrack;
    int m_Width, m_Height;
    ftdgl::viewport::viewport_s m_Viewport;
    ftdgl::render::RenderPtr m_Render;

    void Init();
    void DoDraw();
    void DrawContent(ftdgl::text::TextBufferPtr buf,
                     std::wstring & content,
                     std::bitset<16> & buffer_mode,
                     uint32_t & last_fore_color,
                     uint32_t & last_back_color,
                     uint32_t & last_mode,
                     uint32_t fore_color,
                     uint32_t back_color,
                     uint32_t mode,
                     float & last_x,
                     float & last_y);
    void InitColorTable();
    void InitFreeTypeGLContext();
    void GetMousePos(uint32_t & row, uint32_t & col);
    ftdgl::text::color_s __GetColorByIndex(uint32_t index);
    void InitViewPort();
};
