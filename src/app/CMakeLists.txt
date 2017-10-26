FILE(GLOB wxglterm_src
  "*.h"
  "*.cpp"
  )

ADD_CUSTOM_TARGET(copy_py_plugins ALL DEPENDS ${CMAKE_BINARY_DIR}/pyplugins.stamp wxglterm)

ADD_EXECUTABLE(wxglterm ${wxglterm_src})

TARGET_COMPILE_DEFINITIONS(wxglterm PRIVATE
  ${wxWidgets_DEFINITIONS}
  )
TARGET_INCLUDE_DIRECTORIES(wxglterm PRIVATE
  ${wxWidgets_INCLUDE_DIRS}
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "../controller"
  "../interface"
  "../ui"
  "../utils"
  "../plugins"
  ${CMAKE_CURRENT_BINARY_DIR}
  )
TARGET_LINK_LIBRARIES(wxglterm
  plugins
  wxglterm_ui
  controller
  wxglterm_interface_static
  utils
  ${wxWidgets_LIBRARIES}
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  ${CMAKE_DL_LIBS}
  )

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/pyplugins.stamp
  COMMAND
  ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/pysrc/plugins ${CMAKE_BINARY_DIR}/plugins
  COMMENT
  "Copying python plugins..."
  VERBATIM
  )
