#
SET(input_handler_src
  input_handler_plugin.cpp input_handler_plugin.h
  key_code_map.cpp key_code_map.h
  network_utils.cpp network_utils.h
  input_handler_key.cpp input_handler_mouse.cpp)

ADD_LIBRARY(term_input_handler SHARED ${input_handler_src})

set_target_properties(term_input_handler
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(term_input_handler PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "../../interface"
  "../../utils"
  "../../plugins"
  )

TARGET_LINK_LIBRARIES(term_input_handler PRIVATE
    utils
    plugins
  )

INSTALL(TARGETS term_input_handler DESTINATION share/wxglterm/plugins)
