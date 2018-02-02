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

#include "base64.h"

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

    DefaultTermWindow * plugin = (DefaultTermWindow *)glfwGetWindowUserPointer(window);

    if (!plugin)
        return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
        plugin->OnKeyDown(key, scancode, mods, action == GLFW_REPEAT);
}

static
void charmods_callback(GLFWwindow* window, unsigned int codepoint, int mods)
{
    (void)window;
    (void)codepoint;
    (void)mods;

    DefaultTermWindow * plugin = (DefaultTermWindow *)glfwGetWindowUserPointer(window);

    if (!plugin)
        return;

    plugin->OnChar(codepoint, mods);
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

static
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    DefaultTermWindow * plugin = (DefaultTermWindow *)glfwGetWindowUserPointer(window);

    if (!plugin)
        return;

    plugin->OnMouseWheel(xoffset, yoffset);
}

static
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    DefaultTermWindow * plugin = (DefaultTermWindow *)glfwGetWindowUserPointer(window);

    if (!plugin)
        return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    plugin->OnMouseButton(button, action, mods, xpos, ypos);
}

static
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    DefaultTermWindow * plugin = (DefaultTermWindow *)glfwGetWindowUserPointer(window);

    if (!plugin)
        return;

    plugin->OnMouseMove(xpos, ypos);
}

DefaultTermWindow::DefaultTermWindow()
    : PluginBase("term_gl_window", "opengl terminal window plugin", 1)
    , m_MainDlg {nullptr}
    , m_FreeTypeGLContext {nullptr}
    , m_TextBuffer {nullptr}
    , m_RefreshNow {0}
    , m_ProcessedKey {0}
    , m_ProcessedMod {0}
    , m_SavedMouseButton {-1}
    , m_EnableMouseTrack {false} {

        mat4_set_identity( &m_Projection );
        mat4_set_identity( &m_Model );
        mat4_set_identity( &m_View );
      }

void DefaultTermWindow::Refresh() {
    {
        std::lock_guard<std::mutex> lk(m_RefreshLock);
        m_RefreshNow++;
    }
    glfwPostEmptyEvent();
}


void DefaultTermWindow::InitFreeTypeGLContext() {
    if (!m_FreeTypeGLContext) {
        m_FreeTypeGLContext = freetype_gl_init();

        TermContextPtr context = std::dynamic_pointer_cast<TermContext>(GetPluginContext());

        if (context) {
            AppConfigPtr appConfig = context->GetAppConfig();

            auto font_size = appConfig->GetEntryUInt64("/term/font/size", 16);
            auto font_name = appConfig->GetEntry("/term/font/name", "Monospace");
            auto font_lang = appConfig->GetEntry("/term/font/lang", "zh");

            int height = 0;
            glfwGetFramebufferSize(m_MainDlg, NULL, &height);

            int w_height;
            glfwGetWindowSize(m_MainDlg, NULL, &w_height);

            font_size = ceil((double)font_size / w_height * height);

            m_FreeTypeGLContext->init_font(font_name, font_size, font_lang);
        }
    }
}

void DefaultTermWindow::Show() {
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    int width = mode->width;
    int height = mode->height;

    if (!m_MainDlg) {
        m_MainDlg = glfwCreateWindow( width, height, "wxglTerm",
                                      NULL,
                                      NULL );

        GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);

        glfwSetCursor(m_MainDlg, cursor);

        glfwMakeContextCurrent(m_MainDlg);
        glfwSwapInterval( 1 );

        glfwSetWindowUserPointer(m_MainDlg, this);

        InitFreeTypeGLContext();

        glfwSetFramebufferSizeCallback(m_MainDlg, reshape );
        glfwSetWindowRefreshCallback(m_MainDlg, display );
        glfwSetKeyCallback(m_MainDlg, keyboard );
        glfwSetWindowCloseCallback(m_MainDlg, close);
        glfwSetCharModsCallback(m_MainDlg, charmods_callback);
        glfwSetCursorPosCallback(m_MainDlg, cursor_position_callback);
        glfwSetMouseButtonCallback(m_MainDlg, mouse_button_callback);
        glfwSetScrollCallback(m_MainDlg, scroll_callback);

#ifndef __APPLE__
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            /* Problem: glewInit failed, something is seriously wrong. */
            fprintf( stderr, "Error: %s\n", glewGetErrorString(err) );
        }
        fprintf( stderr, "Using GLEW %s\n", glewGetString(GLEW_VERSION) );
#endif

        InitColorTable();
    } else {
        glfwMakeContextCurrent(m_MainDlg);
        glfwSwapInterval( 1 );
    }

    Init();

    glfwShowWindow(m_MainDlg);
}

void DefaultTermWindow::Close() {
    if (!m_MainDlg) return;

    glfwSetWindowShouldClose( m_MainDlg, GL_TRUE );
}

void DefaultTermWindow::SetWindowTitle(const std::string & title) {
    glfwSetWindowTitle(m_MainDlg, title.c_str());
}

uint32_t DefaultTermWindow::GetColorByIndex(uint32_t index) {
    ftgl::vec4 c = m_ColorTable[index];

#define COLOR(x) ((uint8_t)((x) * 255))

    uint32_t cv = (COLOR(c.r) << 24)
            | (COLOR(c.g) << 16)
            | (COLOR(c.b) << 8)
            | (COLOR(c.a));
#undef COLOR

    return cv;
}

std::string DefaultTermWindow::GetSelectionData() {
    std::string sel_data {};

    auto data = glfwGetClipboardString(m_MainDlg);

    if (data) {
        if (!Base64::Encode(data, &sel_data)) {
            sel_data.clear();
        }
    }

    return sel_data;
}

void DefaultTermWindow::SetSelectionData(const std::string & sel_data) {
    std::string decoded {};

    if (sel_data.size() == 0) {
        decoded.clear();
        return;
    } else {
        if (!Base64::Decode(sel_data, &decoded)) {
            decoded.clear();
        }
    }

    glfwSetClipboardString(m_MainDlg, decoded.c_str());
}

void DefaultTermWindow::OnSize(int width, int height) {

    mat4_set_orthographic( &m_Projection, 0, width, 0, height, -1, 1);

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
              << ", w:" << width
              << ", h:" << height
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


void DefaultTermWindow::Init() {
    if (!m_TextBuffer) {
        m_TextShader = shader_load();
        m_TextBuffer = ftgl::text_buffer_new( );
    }

    auto font_manager = m_FreeTypeGLContext->font_manager;

    glGenTextures( 1, &font_manager->atlas->id );
}

void DefaultTermWindow::UpdateWindow() {
    int refresh_now = 0;

    {
        std::lock_guard<std::mutex> lk(m_RefreshLock);
        refresh_now = m_RefreshNow;
    }

    if (refresh_now && m_MainDlg)
        OnDraw();

    {
        std::lock_guard<std::mutex> lk(m_RefreshLock);
        m_RefreshNow -= refresh_now;
    }
}

void DefaultTermWindow::EnableMouseTrack(bool enable) {
    m_EnableMouseTrack = enable;
}
