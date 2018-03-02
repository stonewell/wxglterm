#
SET(input_handler_src
  input_handler_plugin.cpp input_handler_plugin.h
  key_code_map.cpp key_code_map.h
  network_utils.cpp network_utils.h
  input_handler_key.cpp input_handler_mouse.cpp)

ADD_LIBRARY(scintilla_editor_input_handler SHARED ${input_handler_src})

set_target_properties(scintilla_editor_input_handler
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(scintilla_editor_input_handler PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "../../interface"
  "../../utils"
  "../../plugins"
  )

TARGET_LINK_LIBRARIES(scintilla_editor_input_handler
  utils
  plugins
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )

INSTALL(TARGETS scintilla_editor_input_handler DESTINATION share/wxglterm/plugins)
