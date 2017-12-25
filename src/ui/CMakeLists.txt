FILE(GLOB wxglterm_ui_src
    "*.h"
    "*.cpp"
    )

ADD_LIBRARY(wxglterm_ui ${wxglterm_ui_src})

TARGET_COMPILE_DEFINITIONS(wxglterm_ui PRIVATE
    ${wxWidgets_DEFINITIONS}
    )
TARGET_INCLUDE_DIRECTORIES(wxglterm_ui PRIVATE
    ${wxWidgets_INCLUDE_DIRS}
    "${pybind11_INCLUDE_DIRS}"
    "${PYTHON_INCLUDE_DIRS}"
    "../controller"
    "../interface"
    "../utils"
    )
