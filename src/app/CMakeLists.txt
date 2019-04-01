FILE(GLOB wxglterm_src
  "*.h"
  "*.cpp"
  )

ADD_CUSTOM_TARGET(copy_py_plugins
    ALL
    DEPENDS
    ${CMAKE_BINARY_DIR}/pyplugins.stamp
    wxglterm)

ADD_EXECUTABLE(wxglterm
    ${wxglterm_src}
    test.cpp
    app_utils.inc)

ADD_CUSTOM_COMMAND(OUTPUT app_utils.inc
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/pystmt_generator/pystmt_generator.py -i ${CMAKE_CURRENT_SOURCE_DIR}/app_utils.py -o app_utils.inc
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/app_utils.py ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/pystmt_generator/pystmt_generator.py
  )

TARGET_COMPILE_DEFINITIONS(wxglterm PRIVATE
  )
TARGET_INCLUDE_DIRECTORIES(wxglterm PRIVATE
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  "../controller"
  "../interface"
  "../utils"
  "../plugins"
  ${CMAKE_CURRENT_BINARY_DIR}
  )
TARGET_LINK_LIBRARIES(wxglterm
  plugins
  controller
  wxglterm_interface_static
  utils
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

INSTALL(TARGETS wxglterm DESTINATION bin)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/pysrc/plugins DESTINATION share/wxglterm)
