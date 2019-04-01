SET(term_network_win32_console_src
  win32_con_pty.cpp
  pseudo_console_api.cpp pseudo_console_api.h
  term_network_win32_console_plugin.cpp)

ADD_LIBRARY(term_network_win32_console SHARED ${term_network_win32_console_src})

set_target_properties(term_network_win32_console
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(term_network_win32_console PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "../../interface"
  "../../utils"
  "../../plugins"
  "../../utils/portable_thread/includes"
  )

TARGET_LINK_LIBRARIES(term_network_win32_console
  utils
  plugins
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )

TARGET_COMPILE_DEFINITIONS(term_network_win32_console PRIVATE
  "-D_WIN32_WINNT=0x0600"
  )

INSTALL(TARGETS term_network_win32_console DESTINATION share/wxglterm/plugins)
