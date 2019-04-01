SET(term_buffer_src
default_term_line.h
default_term_buffer.cpp       default_term_selection_decl.h
default_term_buffer_decl.h
default_term_buffer.h
default_term_buffer_impl.cpp  internal_term_buffer.cpp
default_term_cell.cpp         internal_term_buffer.h
default_term_cell.h
default_term_line.cpp
default_term_buffer_clone_impl.cpp
  )

ADD_LIBRARY(term_buffer SHARED ${term_buffer_src})

set_target_properties( term_buffer
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(term_buffer PRIVATE
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "../../interface"
  "../../utils"
  "../../utils/portable_thread/includes"
  "../../plugins"
  )

TARGET_LINK_LIBRARIES(term_buffer
  utils
  plugins
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )

INSTALL(TARGETS term_buffer DESTINATION share/wxglterm/plugins)
