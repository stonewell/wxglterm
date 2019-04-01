FIND_PACKAGE(wxWidgets 3 REQUIRED)

IF (NOT APPLE AND NOT WIN32)
   FIND_PACKAGE(FontConfig REQUIRED)
   FIND_PACKAGE(Freetype REQUIRED)
   FIND_PACKAGE(Cairo REQUIRED)
   SET(EXTRA_LIBRARY
	${Cairo_LIBRARY}
	${FREETYPE_LIBRARIES}
	${FONTCONFIG_LIBRARY}
	)
   SET(EXTRA_INCLUDE
	${FREETYPE_INCLUDE_DIRS}
	)
ELSEIF (WIN32)
    # make sure the uuid and oleacc in order
    # add uxtheme lib
    LIST(REMOVE_DUPLICATES wxWidgets_LIBRARIES)
    LIST(REMOVE_ITEM wxWidgets_LIBRARIES oleacc)
    LIST(APPEND wxWidgets_LIBRARIES uuid oleacc uxtheme)
ENDIF()


ADD_SUBDIRECTORY(ui)

SET(term_ui_src
  term_ui_plugin.cpp
  default_term_ui.cpp default_term_ui.h
  )

ADD_LIBRARY(term_ui SHARED ${term_ui_src})

set_target_properties( term_ui
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
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
  ${EXTRA_INCLUDE}
  )

TARGET_LINK_LIBRARIES(term_ui PRIVATE
    utils
    plugins
    wxglterm_ui
    ${wxWidgets_LIBRARIES}
    ${EXTRA_LIBRARY}
  )

INSTALL(TARGETS term_ui DESTINATION share/wxglterm/plugins)
