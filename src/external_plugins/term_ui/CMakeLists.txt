#
FIND_PACKAGE(wxWidgets 3 REQUIRED
  std
  richtext
  )
FIND_PACKAGE(Cairo REQUIRED)

ADD_SUBDIRECTORY(ui)

SET(term_ui_src
  term_ui_plugin.cpp
  default_term_ui.cpp default_term_ui.h
  )

ADD_LIBRARY(term_ui SHARED ${term_ui_src})

set_target_properties( term_ui
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

TARGET_COMPILE_DEFINITIONS(term_ui PRIVATE
  ${wxWidgets_DEFINITIONS}
  )

TARGET_INCLUDE_DIRECTORIES(term_ui PRIVATE
  "${wxWidgets_INCLUDE_DIRS}"
  "${CMAKE_CURRENT_SRC_DIR}"
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "ui"
  "ui/text_blob"
  "../../interface"
  "../../utils"
  "../../utils/portable_thread/includes"
  "../../plugins"
  )

TARGET_LINK_LIBRARIES(term_ui
  utils
  plugins
  wxglterm_ui
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${wxWidgets_LIBRARIES}
  ${Cairo_LIBRARY}
  )

INSTALL(TARGETS term_ui DESTINATION share/wxglterm/plugins)
