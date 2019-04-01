#
SET(term_network_pty_src
  term_network_pty_plugin.cpp)

FIND_LIBRARY(UTIL_LIB util REQUIRED)

ADD_LIBRARY(term_network_pty SHARED ${term_network_pty_src})

set_target_properties( term_network_pty
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(term_network_pty PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "../../interface"
  "../../utils"
  "../../plugins"
  "../../utils/portable_thread/includes"
  )

TARGET_LINK_LIBRARIES(term_network_pty
  utils
  plugins
  ${UTIL_LIB}
  )

INSTALL(TARGETS term_network_pty DESTINATION share/wxglterm/plugins)
