#include <pybind11/embed.h>

#include "default_term_window.h"

#include "term_buffer.h"
#include "term_context.h"
#include "term_network.h"
#include "color_theme.h"

#include "char_width.h"

#include "shader.h"

#include <iostream>
#include <iterator>
#include <functional>
#include <locale>
#include <codecvt>

#define PADDING (5)

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

    auto background_color = m_ColorTable[TermCell::DefaultBackColorIndex];

    glClearColor(background_color.r,
                 background_color.g,
                 background_color.b,
                 background_color.a);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glColor4f(background_color.r,
              background_color.g,
              background_color.b,
              background_color.a);

    glUseProgram( m_TextShader );
    {
        glUniformMatrix4fv( glGetUniformLocation( m_TextShader, "model" ),
                            1, 0, m_Model.data);
        glUniformMatrix4fv( glGetUniformLocation( m_TextShader, "view" ),
                            1, 0, m_View.data);
        glUniformMatrix4fv( glGetUniformLocation( m_TextShader, "projection" ),
                            1, 0, m_Projection.data);
        glUniform1i( glGetUniformLocation( m_TextShader, "tex" ), 0 );
        glUniform3f( glGetUniformLocation( m_TextShader, "pixel" ),
                     1.0f/font_manager->atlas->width,
                     1.0f/font_manager->atlas->height,
                     (float)font_manager->atlas->depth );

        glEnable( GL_BLEND );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, font_manager->atlas->id );

        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glBlendColor(background_color.r,
                     background_color.g,
                     background_color.b,
                     background_color.a);

        vertex_buffer_render( m_TextBuffer->buffer, GL_TRIANGLES );
        glBindTexture( GL_TEXTURE_2D, 0 );
        glBlendColor( 0, 0, 0, 0 );
        glUseProgram( 0 );
    }

    glfwSwapBuffers(m_MainDlg);
}

void DefaultTermWindow::DoDraw() {
    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    TermBufferPtr buffer = context->GetTermBuffer();

    if (!buffer)
        return;

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

    if (m_TextBuffer)
        ftgl::text_buffer_delete(m_TextBuffer);

    m_TextBuffer = ftgl::text_buffer_new( );

    auto rows = buffer->GetRows();
    auto cols = buffer->GetCols();

    int width = 0, height = 0;
    glfwGetFramebufferSize(m_MainDlg, &width, &height);

    uint16_t last_fore_color = TermCell::DefaultForeColorIndex;
    uint16_t last_back_color = TermCell::DefaultBackColorIndex;
    float last_y = height - PADDING;
    float last_x = PADDING;
    uint16_t last_mode = 0;
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
            uint16_t fore_color = TermCell::DefaultForeColorIndex;
            uint16_t back_color = TermCell::DefaultBackColorIndex;
            uint16_t mode = 0;

            if (cell && cell->GetChar() != 0) {
                fore_color = cell->GetForeColorIndex();
                back_color = cell->GetBackColorIndex();
                mode = cell->GetMode();
                ch = cell->GetChar();
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

    auto font_manager = m_FreeTypeGLContext->font_manager;
    glBindTexture( GL_TEXTURE_2D, font_manager->atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, font_manager->atlas->width,
                  font_manager->atlas->height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                  font_manager->atlas->data );

    if (cell)
        cell->RemoveMode(TermCell::Cursor);
}

void DefaultTermWindow::DrawContent(ftgl::text_buffer_t * buf,
                                    std::wstring & content,
                                    std::bitset<16> & buffer_mode,
                                    uint16_t & last_fore_color,
                                    uint16_t & last_back_color,
                                    uint16_t & last_mode,
                                    uint16_t fore_color,
                                    uint16_t back_color,
                                    uint16_t mode,
                                    float & last_x,
                                    float & last_y) {
    std::bitset<16> m(last_mode);

    uint16_t back_color_use = last_back_color;
    uint16_t fore_color_use = last_fore_color;

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
        uint16_t tmp = back_color_use;
        back_color_use = fore_color_use;
        fore_color_use = tmp;
    }

    font.foreground_color = m_ColorTable[fore_color_use];
    font.background_color = m_ColorTable[back_color_use];

    std::string bytes = wcharconv.to_bytes(content);

    ftgl::vec2 pen = {{last_x, last_y}};

    ftgl::text_buffer_printf(buf,
                             &pen,
                             &font,
                             bytes.c_str(),
                             NULL);

    content.clear();

    last_x = pen.x;
    last_y = pen.y;

    last_fore_color = fore_color;
    last_back_color = back_color;
    last_mode = mode;
}
