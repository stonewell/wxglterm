SET(plugins_src
default_term_line.h
default_term_buffer.cpp       default_term_selection_decl.h
default_term_buffer_decl.h
default_term_buffer.h
default_term_buffer_impl.cpp  internal_term_buffer.cpp
default_term_cell.cpp         internal_term_buffer.h
default_term_cell.h           plugin_base.cpp
default_term_line.cpp         plugin_base.h
default_term_buffer_clone_impl.cpp
default_show_context_window_task.cpp default_show_context_window_task.h
)

ADD_LIBRARY(plugins STATIC ${plugins_src})

TARGET_COMPILE_DEFINITIONS(plugins PRIVATE
    ${wxWidgets_DEFINITIONS}
    )
TARGET_INCLUDE_DIRECTORIES(plugins PRIVATE
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  ${wxWidgets_INCLUDE_DIRS}
  "../interface"
  "../utils"
  "../ui"
)
