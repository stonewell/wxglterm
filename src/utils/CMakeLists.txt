FILE(GLOB utils_src
    "*.h"
    "*.cpp"
    )

IF (WIN32)
  FILE(GLOB utils_platform_src
    "platform_specific/windows/*.h"
    "platform_specific/windows/*.cpp"
    "abc.cpp"
    )
ELSE()
  FILE(GLOB utils_platform_src
    "platform_specific/unix_like/*.h"
    "platform_specific/unix_like/*.cpp"
    "abc.cpp"
    )
ENDIF()

ADD_LIBRARY(utils STATIC
  ${utils_src}
  ${utils_platform_src}
  )

TARGET_INCLUDE_DIRECTORIES(utils PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
)
