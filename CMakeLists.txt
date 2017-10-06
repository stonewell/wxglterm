CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT (wxglTerm)

IF(APPLE)
  SET(CMAKE_MACOSX_RPATH 1)
  SET(CMAKE_PREFIX_PATH /usr/local)
ENDIF()

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "DEBUG")
  #SET(CMAKE_BUILD_TYPE "RELEASE")
  #SET(CMAKE_BUILD_TYPE "RELWITHDEBINFO")
  #SET(CMAKE_BUILD_TYPE "MINSIZEREL")
ENDIF()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(warnings "-Wall -Wextra -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(warnings "-Wall -Wextra -Werror -Wno-inconsistent-missing-override")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()

if (NOT CONFIGURED_ONCE)
    set(CMAKE_CXX_FLAGS "-std=c++14 ${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_C_FLAGS   "${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

FIND_PACKAGE(pybind11 REQUIRED)

FIND_PACKAGE(PythonLibs 3 REQUIRED)

FIND_PACKAGE(wxWidgets 3 REQUIRED
  std
  richtext
  )

configure_file(wxglterm.json.in wxglterm.json)

ADD_SUBDIRECTORY(src/interface)
ADD_SUBDIRECTORY(src/controller)
ADD_SUBDIRECTORY(src/ui)
ADD_SUBDIRECTORY(src/utils)
ADD_SUBDIRECTORY(src/plugins)
ADD_SUBDIRECTORY(src/app)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/termdata.stamp
  COMMAND
  ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
  COMMENT
  "Copying term data..."
  VERBATIM
  )

ADD_CUSTOM_TARGET(copy_term_data ALL DEPENDS ${CMAKE_BINARY_DIR}/termdata.stamp wxglterm)
