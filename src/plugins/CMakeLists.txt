SET(plugins_src
  plugin_base.cpp plugin_base.h
  default_show_context_window_task.cpp default_show_context_window_task.h
)

ADD_LIBRARY(plugins STATIC ${plugins_src})

TARGET_INCLUDE_DIRECTORIES(plugins PRIVATE
  "../interface"
  "../utils"
)

TARGET_LINK_LIBRARIES(plugins PRIVATE
    pybind11::embed
    )
