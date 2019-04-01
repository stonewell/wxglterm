#
SET(term_data_handler_src
  term_data_handler_plugin.cpp
  term_data_handler_impl.cpp
  term_data_handler_impl.h
  term_data_handler_impl_decl.h
  term_data_handler_impl_native.cpp)

SET(PY_STMT_GENERATOR
  ${CMAKE_CURRENT_SOURCE_DIR}/../../../utils/pystmt_generator/pystmt_generator.py)

ADD_CUSTOM_COMMAND(OUTPUT term_data_handler_impl.inc
  COMMAND ${PYTHON_EXECUTABLE} ${PY_STMT_GENERATOR} -i ${CMAKE_CURRENT_SOURCE_DIR}/term_data_handler_impl.py -o term_data_handler_impl.inc
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/term_data_handler_impl.py ${PY_STMT_GENERATOR}
  )

ADD_LIBRARY(term_data_handler SHARED
  ${term_data_handler_src}
  term_data_handler_impl.inc
  $<TARGET_OBJECTS:caps>
  )

set_target_properties( term_data_handler
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(term_data_handler PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "../../interface"
  "../../utils"
  "../../caps"
  "../../plugins"
  "../../utils/portable_thread/includes"
  )

TARGET_LINK_LIBRARIES(term_data_handler
  utils
  plugins
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )

INSTALL(TARGETS term_data_handler DESTINATION share/wxglterm/plugins)
