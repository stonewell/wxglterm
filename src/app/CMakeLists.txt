FILE(GLOB wxglterm_src
    "*.h"
    "*.cpp"
    )

  ADD_CUSTOM_COMMAND(OUTPUT app_config.inc
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/pystmt_generator/pystmt_generator.py -i ${CMAKE_CURRENT_SOURCE_DIR}/app_config.py -o app_config.inc
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/app_config.py ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/pystmt_generator/pystmt_generator.py
    )

ADD_EXECUTABLE(wxglterm ${wxglterm_src} app_config.inc)

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
    ${CMAKE_CURRENT_BINARY_DIR}
    )
TARGET_LINK_LIBRARIES(wxglterm
  wxglterm_ui
  controller
  wxglterm_interface_static
  ${wxWidgets_LIBRARIES}
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )
