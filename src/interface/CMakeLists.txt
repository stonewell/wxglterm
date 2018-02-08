FILE(GLOB wxglterm_interface_src
    "*.h"
    "*.cpp"
    )

ADD_LIBRARY(wxglterm_interface SHARED ${wxglterm_interface_src})
ADD_LIBRARY(wxglterm_interface_static ${wxglterm_interface_src})

ADD_DEPENDENCIES(wxglterm_interface_static pybind11)
ADD_DEPENDENCIES(wxglterm_interface pybind11)

SET_TARGET_PROPERTIES(wxglterm_interface PROPERTIES
    PREFIX "${PYTHON_MODULE_PREFIX}"
    SUFFIX "${PYTHON_MODULE_EXTENSION}"
    )

TARGET_COMPILE_DEFINITIONS(wxglterm_interface PRIVATE
    WXGLTERM_DYNAMIC_INTERFACE
    )

TARGET_INCLUDE_DIRECTORIES(wxglterm_interface PRIVATE
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  )

TARGET_INCLUDE_DIRECTORIES(wxglterm_interface_static PRIVATE
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  )

TARGET_LINK_LIBRARIES(wxglterm_interface
	${pybind11_LIBRARIES}
	${PYTHON_LIBRARIES}
	)
