#
SET(scintilla_editor_data_handler_src
  scintilla_editor_plugin.cpp
  scintilla_editor_impl.cpp
  scintilla_editor_impl.h
  scintilla_editor_impl_decl.h
  scintilla_editor_impl_native.cpp)

ADD_LIBRARY(scintilla_editor_data_handler SHARED
  ${scintilla_editor_data_handler_src}
  )

set_target_properties( scintilla_editor_data_handler
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_INCLUDE_DIRECTORIES(scintilla_editor_data_handler PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "../../interface"
  "../../utils"
  "../../caps"
  "../../plugins"
  "../../utils/portable_thread/includes"
  )

TARGET_LINK_LIBRARIES(scintilla_editor_data_handler
  utils
  plugins
  $<TARGET_OBJECTS:caps>
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )

INSTALL(TARGETS scintilla_editor_data_handler DESTINATION share/wxglterm/plugins)
