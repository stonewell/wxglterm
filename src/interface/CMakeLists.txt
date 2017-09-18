FIND_PACKAGE(pybind11 REQUIRED)

FIND_PACKAGE(PythonLibs 3 REQUIRED)

FILE(GLOB wxglterm_interface_src
    "*.h"
    "*.cpp"
    )

ADD_DEFINITIONS(-Wall
    )

ADD_LIBRARY(wxglterm_interface SHARED ${wxglterm_interface_src})

SET_TARGET_PROPERTIES(wxglterm_interface PROPERTIES
    PREFIX "${PYTHON_MODULE_PREFIX}"
    SUFFIX "${PYTHON_MODULE_EXTENSION}"
    )

TARGET_INCLUDE_DIRECTORIES(wxglterm_interface PRIVATE
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  )

TARGET_LINK_LIBRARIES(wxglterm_interface
	${pybind11_LIBRARIES}
	${PYTHON_LIBRARIES}
	)
