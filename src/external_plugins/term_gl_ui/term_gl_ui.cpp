#include <pybind11/embed.h>

#include "plugin.h"
#include "plugin_manager.h"
#include "term_gl_ui.h"
#include "term_window.h"
#include "task.h"
#include "term_context.h"
#include "term_network.h"
#include "term_buffer.h"
#include "term_data_handler.h"
#include "app_config_impl.h"
#include "plugin_base.h"

#include <iostream>

#include "freetype-gl.h"
#include "vertex-buffer.h"
#include "text-buffer.h"

#include <GLFW/glfw3.h>

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

    ~__GLTermUIInitializer() {
        if (init)
            glfwTerminate();
    }
};

class DefaultTermUI : public virtual PluginBase, public virtual TermUI {
    static __GLTermUIInitializer _initializer;
public:
    DefaultTermUI() :
        PluginBase("term_gl_ui", "opengl terminal ui plugin", 1)
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

    void ProcessTasks() {
        auto cur_time = glfwGetTime();

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
            }
        }
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

    int32_t StartMainUILoop() {
        glfwWindowHint( GLFW_VISIBLE, GL_FALSE );
        glfwWindowHint( GLFW_RESIZABLE, GL_TRUE );

        pybind11::gil_scoped_release release;

        while (!AllWindowClosed())
        {
            glfwPollEvents( );
            ProcessTasks();
        }

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

        return true;
    }
};

__GLTermUIInitializer DefaultTermUI::_initializer;

TermUIPtr CreateOpenGLTermUI() {
    return TermUIPtr{ new DefaultTermUI()};
}
