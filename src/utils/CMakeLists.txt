FILE(GLOB utils_src
    "*.h"
    "*.cpp"
    )

ADD_LIBRARY(utils STATIC ${utils_src})

TARGET_INCLUDE_DIRECTORIES(utils PRIVATE
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
)
