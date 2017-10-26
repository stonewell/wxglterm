#
SET(term_data_handler_src
  term_data_handler_plugin.cpp
  term_data_handler_impl.cpp
  term_data_handler_impl.h)

ADD_LIBRARY(term_data_handler SHARED ${term_data_handler_src})

set_target_properties( term_data_handler
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(term_data_handler PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "../../interface"
  "../../plugins"
  "../../utils"
  "../../utils/portable_thread/includes"
  )

TARGET_LINK_LIBRARIES(term_data_handler
  plugins
  utils
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )
