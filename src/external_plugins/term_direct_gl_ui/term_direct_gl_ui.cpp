#include <pybind11/embed.h>

#include "plugin.h"
#include "plugin_manager.h"
#include "term_direct_gl_ui.h"
#include "term_window.h"
#include "task.h"
#include "term_context.h"
#include "term_network.h"
#include "term_buffer.h"
#include "term_data_handler.h"
#include "app_config_impl.h"
#include "plugin_base.h"

#include <iostream>

#include "font_manager.h"
#include "text_buffer.h"
#include "render.h"

#include "default_term_window.h"


// --------------------------------------------------------- error-callback ---
void error_callback( int error, const char* description )
{
    std::cerr << "error:" << error << ", description:" << description << std::endl;
}

class __GLTermUIInitializer {
public:
    bool init;

    __GLTermUIInitializer() : init(false) {
    }

    void Initialize() {
        if (init) return;

        glfwSetErrorCallback( error_callback );

        init = glfwInit();
    }

    void Cleanup() {
        if (init)
            glfwTerminate();
        init = false;
    }

    ~__GLTermUIInitializer() {
    }
};

class DefaultTermUI : public virtual PluginBase, public virtual TermUI {
    static __GLTermUIInitializer _initializer;
public:
    DefaultTermUI() :
        PluginBase("term_direct_gl_ui", "freetype direct opengl terminal ui plugin", 1)
    {
    }

    virtual ~DefaultTermUI() {
    }

    struct TaskEntry {
        TaskPtr task;
        double end_time;
        bool repeated;
        bool done;
        int interval;
    };

    std::vector<TaskEntry> m_Tasks;
    std::vector<TermWindowPtr> m_Windows;

    TermWindowPtr CreateWindow() {
        DefaultTermUI::_initializer.Initialize();

        auto window = TermWindowPtr { new DefaultTermWindow() };
        window->InitPlugin(GetPluginContext(),
                           GetPluginConfig());

        m_Windows.push_back(window);
        return window;
    }

    double ProcessTasks() {
        auto cur_time = glfwGetTime();

        double next_end_time = 0.0;

        for(auto & entry : m_Tasks) {
            if (entry.done) continue;

            if (cur_time * 1000 >= entry.end_time) {
                if (entry.task && !entry.task->IsCancelled()) {
                    entry.task->Run();
                }

                if (entry.repeated) {
                    entry.end_time = cur_time * 1000 + entry.interval;
                } else {
                    entry.done = true;
                }
            } else if (next_end_time == 0.0
                       || entry.end_time < next_end_time) {
                next_end_time = entry.end_time;
            }
        }

        return next_end_time == 0.0 ? 0.0 : (next_end_time - cur_time);
    }

    bool AllWindowClosed() {
        for(auto & window : m_Windows) {
            DefaultTermWindow * pWindow =
                    dynamic_cast<DefaultTermWindow*>(window.get());

            if (!pWindow->ShouldClose())
                return false;
        }

        return true;
    }

    void UpdateAllWindows() {
        for(auto & window : m_Windows) {
            DefaultTermWindow * pWindow =
                    dynamic_cast<DefaultTermWindow*>(window.get());

            pWindow->UpdateWindow();
        }
    }

    int32_t StartMainUILoop() {
        glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
        glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        pybind11::gil_scoped_release release;

        double delta = ProcessTasks();

        while (!AllWindowClosed())
        {
            if (delta) {
                glfwWaitEventsTimeout(delta / 1000);
            } else {
                glfwWaitEvents();
            }
            delta = ProcessTasks();
            UpdateAllWindows();
        }

        DefaultTermUI::_initializer.Cleanup();
        return 0;
    }

    bool ScheduleTask(TaskPtr task, int miliseconds, bool repeated) {
        TaskEntry entry {
            .task = task,
                    .end_time = glfwGetTime() * 1000 + miliseconds,
                    .repeated = repeated,
                    .done = false,
                    .interval = miliseconds
                    };

        m_Tasks.push_back(entry);

        glfwPostEmptyEvent();
        return true;
    }
};

__GLTermUIInitializer DefaultTermUI::_initializer;

TermUIPtr CreateOpenGLTermUI() {
    return TermUIPtr{ new DefaultTermUI()};
}
