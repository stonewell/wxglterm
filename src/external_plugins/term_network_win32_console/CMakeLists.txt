SET(term_network_win32_console_src
  win32_con_pty.cpp
  pseudo_console_api.cpp pseudo_console_api.h
  term_network_win32_console_plugin.cpp
  win32_console.h win32_console.cpp
  win32_con_pty_console.h win32_con_pty_console.cpp)

ADD_LIBRARY(term_network_win32_console SHARED ${term_network_win32_console_src})

include(GenerateExportHeader)
generate_export_header(term_network_win32_console)

set_target_properties(term_network_win32_console
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(term_network_win32_console PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  "../../interface"
  "../../utils"
  "../../plugins"
  "../../utils/portable_thread/includes"
  )

TARGET_LINK_LIBRARIES(term_network_win32_console
  utils
  plugins
  )

TARGET_COMPILE_DEFINITIONS(term_network_win32_console PRIVATE
  "-D_WIN32_WINNT=0x0600"
  )

INSTALL(TARGETS term_network_win32_console DESTINATION share/wxglterm/plugins)
