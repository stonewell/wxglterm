#
SET(input_handler_src
  input_handler_plugin.cpp)

ADD_LIBRARY(term_input_handler SHARED ${input_handler_src})

set_target_properties(term_input_handler
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(term_input_handler PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "../../interface"
  "../../utils"
  "../../plugins"
  )

TARGET_LINK_LIBRARIES(term_input_handler
  utils
  plugins
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )

INSTALL(TARGETS term_input_handler DESTINATION share/wxglterm/plugins)
