FILE(GLOB controller_src
    "*.h"
    "*.cpp"
    )

ADD_LIBRARY(controller STATIC ${controller_src})

TARGET_INCLUDE_DIRECTORIES(controller PRIVATE
  "../interface"
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
)
