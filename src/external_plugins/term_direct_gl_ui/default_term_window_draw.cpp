#include <pybind11/embed.h>

#include "default_term_window.h"

#include "term_buffer.h"
#include "term_context.h"
#include "term_network.h"
#include "color_theme.h"

class __ScopeLocker {
public:
    __ScopeLocker(TermBufferPtr termBuffer) :
        m_TermBuffer(termBuffer)
    {
        m_TermBuffer->LockUpdate();
    }

    ~__ScopeLocker() {
        m_TermBuffer->UnlockUpdate();
    }

    TermBufferPtr m_TermBuffer;
};

#include "char_width.h"

#include <iostream>
#include <iterator>
#include <functional>
#include <locale>
#include <codecvt>
#include <unordered_set>
static
std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> wcharconv;

static
bool contains(const std::vector<uint32_t> & rowsToDraw, uint32_t row) {
    for(auto & it : rowsToDraw) {
        if (it == row)
            return true;
    }

    return false;
}

void DefaultTermWindow::OnDraw() {
    DoDraw();

    auto font_manager = m_FreeTypeGLContext->font_manager;

    auto background_color = __GetColorByIndex(TermCell::DefaultBackColorIndex);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(background_color.r,
                 background_color.g,
                 background_color.b,
                 background_color.a);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glColor4f(background_color.r,
              background_color.g,
              background_color.b,
              background_color.a);

    glViewport(0, 0, m_Viewport.pixel_width, m_Viewport.pixel_height);
    m_Render->RenderText(m_TextBuffer);

    glfwSwapBuffers(m_MainDlg);
}

void DefaultTermWindow::DoDraw() {
    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    TermBufferPtr buffer = context->GetTermBuffer();

    if (!buffer)
        return;

    //__ScopeLocker buffer_locker(buffer);

    TermCellPtr cell = buffer->GetCurCell();

    if (cell) {
        if (cell->GetChar() == 0 && buffer->GetCol() > 0) {
            cell = buffer->GetCell(buffer->GetRow(), buffer->GetCol() - 1);
        }

        if (cell) {
            cell->AddMode(TermCell::Cursor);
        }
    }
    else {
        TermLinePtr line = buffer->GetCurLine();

        if (line)
            line->SetModified(true);
    }

    m_TextBuffer->Clear();

    auto rows = buffer->GetRows();
    auto cols = buffer->GetCols();

    int width = 0, height = 0;
    glfwGetWindowSize(m_MainDlg, &width, &height);

    uint32_t last_fore_color = TermCell::DefaultForeColorIndex;
    uint32_t last_back_color = TermCell::DefaultBackColorIndex;
    float last_y = height - PADDING;
    float last_x = PADDING;
    uint32_t last_mode = 0;
    std::wstring content{L""};

    //place hold for parameters
    bool full_paint = true;
    std::vector<uint32_t> rowsToDraw;

    std::bitset<16> m(buffer->GetMode());

    for (auto row = 0u; row < rows; row++) {
        auto line = buffer->GetLine(row);

        if ((!full_paint &&
             row == line->GetLastRenderLineIndex()
             && !line->IsModified())
            || (rowsToDraw.size() > 0 && !contains(rowsToDraw, row)))
        {
            if (content.size() > 0)
            {
                //append \n to make sure the last_y moving to next line
                content.append(L"\n");
                DrawContent(m_TextBuffer,
                            content,
                            m,
                            last_fore_color,
                            last_back_color,
                            last_mode,
                            TermCell::DefaultForeColorIndex,
                            TermCell::DefaultBackColorIndex,
                            0,
                            last_x,
                            last_y);
            }

            last_x = PADDING;

            continue;
        }

        line->SetLastRenderLineIndex(row);
        line->SetModified(false);

        for (auto col = 0u; col < cols; col++) {
            auto cell = line->GetCell(col);

            wchar_t ch = 0;
            uint32_t fore_color = TermCell::DefaultForeColorIndex;
            uint32_t back_color = TermCell::DefaultBackColorIndex;
            uint32_t mode = 0;

            if (cell && (ch = cell->GetChar()) != 0) {
                fore_color = cell->GetForeColorIndex();
                back_color = cell->GetBackColorIndex();
                mode = cell->GetMode();
            } else if (!cell) {
                ch = ' ';
            }

            if (ch != 0)
            {
                if (last_fore_color != fore_color
                    || last_back_color != back_color
                    || last_mode != mode)
                {
                    DrawContent(m_TextBuffer,
                                content,
                                m,
                                last_fore_color,
                                last_back_color,
                                last_mode,
                                fore_color,
                                back_color,
                                mode,
                                last_x,
                                last_y);
                }

                content.append(&ch, 1);
            }

        }

        if (last_x == PADDING)
        {
            if (row != rows - 1)
                content.append(L"\n");
        }
        else if (content.size() > 0)
        {
            //append \n to make sure the last_y moving to next line
            content.append(L"\n");
            DrawContent(m_TextBuffer,
                        content,
                        m,
                        last_fore_color,
                        last_back_color,
                        last_mode,
                        TermCell::DefaultForeColorIndex,
                        TermCell::DefaultBackColorIndex,
                        0,
                        last_x,
                        last_y);

            last_x = PADDING;
        }
    }

    if (content.size() > 0)
    {
        DrawContent(m_TextBuffer,
                    content,
                    m,
                    last_fore_color,
                    last_back_color,
                    last_mode,
                    TermCell::DefaultForeColorIndex,
                    TermCell::DefaultBackColorIndex,
                    0,
                    last_x,
                    last_y);
    }

    if (cell)
        cell->RemoveMode(TermCell::Cursor);
}

void DefaultTermWindow::DrawContent(ftdgl::text::TextBufferPtr buf,
                                    std::wstring & content,
                                    std::bitset<16> & buffer_mode,
                                    uint32_t & last_fore_color,
                                    uint32_t & last_back_color,
                                    uint32_t & last_mode,
                                    uint32_t fore_color,
                                    uint32_t back_color,
                                    uint32_t mode,
                                    float & last_x,
                                    float & last_y) {
    std::bitset<16> m(last_mode);

    uint32_t back_color_use = last_back_color;
    uint32_t fore_color_use = last_fore_color;

    auto font {*m_FreeTypeGLContext->get_font(FontCategoryEnum::Default)};

    if (m.test(TermCell::Bold) ||
        buffer_mode.test(TermCell::Bold)) {
        if (back_color_use < 8)
            back_color_use += 8;

        if (fore_color_use < 8)
            fore_color_use += 8;

        font = *m_FreeTypeGLContext->get_font(FontCategoryEnum::Bold);
    }

    if (m.test(TermCell::Cursor))
    {
        back_color_use = TermCell::DefaultCursorColorIndex;
        fore_color_use = last_back_color;
    }

    if (m.test(TermCell::Reverse) ||
        buffer_mode.test(TermCell::Reverse))
    {
        uint32_t tmp = back_color_use;
        back_color_use = fore_color_use;
        fore_color_use = tmp;
    }

    font.fore_color = __GetColorByIndex(fore_color_use);
    font.back_color = __GetColorByIndex(back_color_use);

    ftdgl::text::pen_s pen = {last_x, last_y};

    buf->AddText(pen,
                 font,
                 content);

    content.clear();

    last_x = pen.x;
    last_y = pen.y;

    last_fore_color = fore_color;
    last_back_color = back_color;
    last_mode = mode;
}

ftdgl::text::color_s DefaultTermWindow::__GetColorByIndex(uint32_t index) {
#define C2V(x) ((float)(x) / 255.0)
    if (index >= TermCell::ColorIndexCount) {
        index -= TermCell::ColorIndexCount;

        return {
                C2V((index >> 16) & 0xFF),
                C2V((index >> 8) & 0xFF),
                C2V((index) & 0xFF),
                C2V(0xFF)
            };
    }

    return m_ColorTable[index];
#undef C2V
}
