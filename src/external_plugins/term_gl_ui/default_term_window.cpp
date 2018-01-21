#include "default_term_window.h"

#include "term_buffer.h"
#include "term_context.h"
#include "term_network.h"

#include "char_width.h"

#include <iostream>

#define PADDING (5)

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
    }

    glfwShowWindow(m_MainDlg);
    glfwMakeContextCurrent(m_MainDlg);
    glfwSwapInterval( 1 );
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

static
bool contains(const std::vector<uint32_t> & rowsToDraw, uint32_t row) {
    for(auto & it : rowsToDraw) {
        if (it == row)
            return true;
    }

    return false;
}

void DefaultTermWindow::OnDraw() {
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
                DrawContent(content,
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
                    DrawContent(content,
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
            DrawContent(content,
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
        DrawContent(content,
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

void DefaultTermWindow::DrawContent(std::wstring & content,
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

    auto font {m_FreeTypeGLContext->get_font(FontCategoryEnum::Default)};

    (void)font;
    if (m.test(TermCell::Bold) ||
        buffer_mode.test(TermCell::Bold)) {
        if (back_color_use < 8)
            back_color_use += 8;

        if (fore_color_use < 8)
            fore_color_use += 8;

        font = m_FreeTypeGLContext->get_font(FontCategoryEnum::Bold);
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

    for(const auto & c : content) {
        if (c == '\n') {
            last_y += m_FreeTypeGLContext->line_height;
            last_x = PADDING;
            continue;
        }

        auto w = char_width(c);

        if (!w || w == (size_t)-1) continue;


        last_x += w * m_FreeTypeGLContext->col_width;
    }

    content.clear();
    last_fore_color = fore_color;
    last_back_color = back_color;
    last_mode = mode;
}
