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

// ---------------------------------------------------------------- reshape ---
static
void reshape( GLFWwindow* window, int width, int height )
{
    (void)window;
    glViewport(0, 0, width, height);

    DefaultTermWindow * plugin = (DefaultTermWindow *)glfwGetWindowUserPointer(window);

    if (!plugin)
        return;

    plugin->OnSize(width, height);
}


// --------------------------------------------------------------- keyboard ---
static
void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
    (void)window;
    (void)key;
    (void)scancode;
    (void)action;
    (void)mods;

    if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
    {
        glfwSetWindowShouldClose( window, GL_TRUE );
    }
}

// ---------------------------------------------------------------- display ---
static
void display( GLFWwindow* window )
{
    (void)window;
    DefaultTermWindow * plugin = (DefaultTermWindow *)glfwGetWindowUserPointer(window);

    if (!plugin)
        return;

    plugin->OnDraw();
}

static
void close( GLFWwindow* window )
{
    glfwSetWindowShouldClose( window, GL_TRUE );
}


void DefaultTermWindow::Refresh() {
}

void DefaultTermWindow::Show() {
    if (!m_MainDlg) {
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        m_MainDlg = glfwCreateWindow( mode->width, mode->height, "wxglTerm",
                                      NULL,
                                      NULL );

        glfwSetFramebufferSizeCallback(m_MainDlg, reshape );
        glfwSetWindowRefreshCallback(m_MainDlg, display );
        glfwSetKeyCallback(m_MainDlg, keyboard );
        glfwSetWindowCloseCallback(m_MainDlg, close);
        glfwSetWindowUserPointer(m_MainDlg, this);

        InitColorTable();
    }

    glfwShowWindow(m_MainDlg);
    glfwMakeContextCurrent(m_MainDlg);
    glfwSwapInterval( 1 );

    Init();
}

void DefaultTermWindow::Close() {
    if (!m_MainDlg) return;

    glfwSetWindowShouldClose( m_MainDlg, GL_TRUE );
    glfwDestroyWindow(m_MainDlg);
}

void DefaultTermWindow::SetWindowTitle(const std::string & title) {
    glfwSetWindowTitle(m_MainDlg, title.c_str());
}

uint32_t DefaultTermWindow::GetColorByIndex(uint32_t index) {
    ftgl::vec4 c = m_ColorTable[index];

#define COLOR(x) ((uint8_t)((x) * 255))

    return (COLOR(c.r) << 24)
            | (COLOR(c.g) << 16)
            | (COLOR(c.b) << 8)
            | (COLOR(c.a));
#undef COLOR
}

std::string DefaultTermWindow::GetSelectionData() {
    std::string sel_data {};

    return sel_data;
}

void DefaultTermWindow::SetSelectionData(const std::string & sel_data) {
    (void)sel_data;
}

void DefaultTermWindow::OnSize(int width, int height) {

    mat4_set_orthographic( &projection, 0, width, 0, height, -1, 1);

    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    TermBufferPtr buffer = context->GetTermBuffer();

    if (!buffer)
        return;

    buffer->Resize((height - PADDING * 2) / m_FreeTypeGLContext->line_height,
                   (width - PADDING * 2) / m_FreeTypeGLContext->col_width);

    std::cout << "row:" << buffer->GetRows()
              << ", cols:" << buffer->GetCols()
              << std::endl;

    TermNetworkPtr network = context->GetTermNetwork();

    if (network)
        network->Resize(buffer->GetRows(),
                        buffer->GetCols());
}

bool DefaultTermWindow::ShouldClose() {
    if (!m_MainDlg)
        return false;
    return glfwWindowShouldClose(m_MainDlg);
}

void DefaultTermWindow::InitColorTable()
{
#define C2V(x) ((float)(x) / 255)
    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());
    TermColorThemePtr color_theme = context->GetTermColorTheme();

    try
    {
        pybind11::gil_scoped_acquire acquire;

        for(int i = 0; i < TermCell::ColorIndexCount;i++)
        {
            TermColorPtr color = color_theme->GetColor(i);

            m_ColorTable[i] = {{C2V(color->r),
                                C2V(color->g),
                                C2V(color->b),
                                C2V(color->a)
                }};
        }
    }
    catch(std::exception & e)
    {
        std::cerr << "!!Error InitColorTable:"
                  << std::endl
                  << e.what()
                  << std::endl;
        PyErr_Print();
    }
    catch(...)
    {
        std::cerr << "!!Error InitColorTable"
                  << std::endl;
        PyErr_Print();
    }

#undef C2V
}

static
bool contains(const std::vector<uint32_t> & rowsToDraw, uint32_t row) {
    for(auto & it : rowsToDraw) {
        if (it == row)
            return true;
    }

    return false;
}

void DefaultTermWindow::OnDraw() {
    auto background_color = m_ColorTable[TermCell::DefaultBackColorIndex];

    glClearColor(background_color.r,
                 background_color.g,
                 background_color.b,
                 background_color.a);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glColor4f(1.00,1.00,1.00,1.00);

    DoDraw();

    auto font_manager = m_FreeTypeGLContext->font_manager;

    glUseProgram( m_TextShader );
    {
        glUniformMatrix4fv( glGetUniformLocation( m_TextShader, "model" ),
                            1, 0, model.data);
        glUniformMatrix4fv( glGetUniformLocation( m_TextShader, "view" ),
                            1, 0, view.data);
        glUniformMatrix4fv( glGetUniformLocation( m_TextShader, "projection" ),
                            1, 0, projection.data);
        glUniform1i( glGetUniformLocation( m_TextShader, "tex" ), 0 );
        glUniform3f( glGetUniformLocation( m_TextShader, "pixel" ),
                     1.0f/font_manager->atlas->width,
                     1.0f/font_manager->atlas->height,
                     (float)font_manager->atlas->depth );

        glEnable( GL_BLEND );

        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, font_manager->atlas->id );

        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glBlendColor( 1, 1, 1, 1 );

        vertex_buffer_render( m_TextBuffer->buffer, GL_TRIANGLES );
        glBindTexture( GL_TEXTURE_2D, 0 );
        glBlendColor( 0, 0, 0, 0 );
        glUseProgram( 0 );
    }

    glfwSwapBuffers(m_MainDlg);
    std::cout << "repaint............" << std::endl;
}

void DefaultTermWindow::DoDraw() {
    TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

    if (!context)
        return;

    TermBufferPtr buffer = context->GetTermBuffer();

    if (!buffer)
        return;

    auto rows = buffer->GetRows();
    auto cols = buffer->GetCols();

    float y = PADDING;

    uint16_t last_fore_color = TermCell::DefaultForeColorIndex;
    uint16_t last_back_color = TermCell::DefaultBackColorIndex;
    float last_y = PADDING;
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
            y += m_FreeTypeGLContext->line_height;

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

            last_x = PADDING;
            last_y = y;

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
                    last_y = y;
                }

                content.append(&ch, 1);
            }

        }

        y += m_FreeTypeGLContext->line_height;

        if (last_x == PADDING)
        {
            if (row != rows - 1)
                content.append(L"\n");
        }
        else if (content.size() > 0)
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

            last_x = PADDING;
            last_y = y;
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

    std::cout << bytes << "," << std::endl << last_x << "," << last_y << std::endl;
}

void DefaultTermWindow::Init() {
    if (!m_TextBuffer) {
        m_TextShader = shader_load();
        m_TextBuffer = ftgl::text_buffer_new( );
    }

    auto font_manager = m_FreeTypeGLContext->font_manager;

    glGenTextures( 1, &font_manager->atlas->id );
    glBindTexture( GL_TEXTURE_2D, font_manager->atlas->id );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, font_manager->atlas->width,
                  font_manager->atlas->height, 0, GL_RGB, GL_UNSIGNED_BYTE,
                  font_manager->atlas->data );

    mat4_set_identity( &projection );
    mat4_set_identity( &model );
    mat4_set_identity( &view );
}
