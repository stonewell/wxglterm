#
SET(term_data_handler_src
  term_data_handler_plugin.cpp
  term_data_handler_impl.cpp
  term_data_handler_impl.h
  cap_manager.cpp
  cap_manager.h
  char_width.cpp
  char_width.h
  cap_cursor.cpp
  cap_attrs.cpp)

SET(PY_STMT_GENERATOR
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../utils/pystmt_generator/pystmt_generator.py)

ADD_CUSTOM_COMMAND(OUTPUT term_data_handler_impl.inc
  COMMAND ${PYTHON_EXECUTABLE} ${PY_STMT_GENERATOR} -i ${CMAKE_CURRENT_SOURCE_DIR}/term_data_handler_impl.py -o term_data_handler_impl.inc
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/term_data_handler_impl.py ${PY_STMT_GENERATOR}
  )

ADD_LIBRARY(term_data_handler SHARED
  ${term_data_handler_src}
  term_data_handler_impl.inc
  )

set_target_properties( term_data_handler
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(term_data_handler PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
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
