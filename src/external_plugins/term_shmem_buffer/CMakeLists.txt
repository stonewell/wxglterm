SET(term_shmem_buffer_src
  term_shmem_buffer.cpp
  term_shmem_buffer_decl.h
  term_shmem_buffer_impl.cpp
  default_term_cell.cpp
  default_term_cell.h
  default_term_line.cpp
  default_term_line.h
  default_term_selection_decl.h
  internal_term_shmem_buffer.cpp
  internal_term_shmem_buffer.h
  )

ADD_LIBRARY(term_shmem_buffer SHARED ${term_shmem_buffer_src})

include(GenerateExportHeader)
generate_export_header(term_shmem_buffer)

set_target_properties(term_shmem_buffer
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/plugins"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/plugins"
  )

TARGET_INCLUDE_DIRECTORIES(term_shmem_buffer PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
  "../../interface"
  "../../utils"
  "../../utils/portable_thread/includes"
  "../../plugins"
  )

TARGET_LINK_LIBRARIES(term_shmem_buffer
  utils
  plugins
  )

INSTALL(TARGETS term_shmem_buffer DESTINATION share/wxglterm/plugins)
