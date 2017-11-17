#
SET(term_network_pty_src
  term_network_pty_plugin.cpp)

ADD_LIBRARY(term_network_pty SHARED ${term_network_pty_src})

set_target_properties( term_network_pty
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(term_network_pty PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "../../interface"
  "../../utils"
  "../../utils/portable_thread/includes"
  )

TARGET_LINK_LIBRARIES(term_network_pty
  utils
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )
