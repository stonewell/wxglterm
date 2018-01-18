#include "default_term_window.h"

#include "term_buffer.h"
#include "term_context.h"
#include "term_network.h"

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
    (void)index;
    return 0;
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

    buffer->Resize((height - PADDING * 2) / 16,
                   (width - PADDING * 2) / 9);

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
