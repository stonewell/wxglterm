FILE(GLOB wxglterm_src
    "*.h"
    "*.cpp"
    )

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
    )
TARGET_LINK_LIBRARIES(wxglterm
  wxglterm_ui
  controller
  wxglterm_interface_static
  ${wxWidgets_LIBRARIES}
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  )
