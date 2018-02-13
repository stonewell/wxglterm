SET(plugins_src
  plugin_base.cpp plugin_base.h
  default_show_context_window_task.cpp default_show_context_window_task.h
)

ADD_LIBRARY(plugins STATIC ${plugins_src})
ADD_DEPENDENCIES(plugins pybind11)

TARGET_INCLUDE_DIRECTORIES(plugins PRIVATE
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "../interface"
  "../utils"
)
