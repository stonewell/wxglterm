CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

CMAKE_POLICY(
    SET CMP0048 NEW
    )

PROJECT (wxglTerm
	VERSION 1.0.0
	)

INCLUDE(ExternalProject)

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

OPTION(BUILD_OPENGL_UI "build opengl ui" ON)
OPTION(BUILD_WXWIDGETS_UI "build wxwidgets ui" ON)
OPTION(BUILD_SCINTILLA_EDITOR "build editor buffer using scintilla" OFF)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(warnings "-Wall -Wextra -Werror")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(warnings "-Wall -Wextra -Werror -Wno-inconsistent-missing-override")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()

SET(CXX_STANDARD_REQUIRED OFF)
SET(CXX_EXTENSION NO)

if (NOT CONFIGURED_ONCE)
    set(CMAKE_C_FLAGS "-fPIC ${warnings}"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
    set(CMAKE_CXX_FLAGS "-fPIC ${warnings} -std=c++14"
        CACHE STRING "Flags used by the compiler during all build types." FORCE)
endif()

EXTERNALPROJECT_ADD(
  pybind11

  GIT_REPOSITORY "https://github.com/pybind/pybind11.git"
  GIT_TAG "master"

  PATCH_COMMAND ""
  UPDATE_COMMAND ""

  CMAKE_ARGS -DCMAKE_BUILD_TYPE=Debug -DPYBIND11_TEST=OFF -DPYBIND11_INSTALL=ON -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/pybind11_install

  TEST_COMMAND ""
)

SET(pybind11_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/pybind11_install/include)

FIND_PACKAGE(PythonInterp 3 REQUIRED)
FIND_PACKAGE(PythonLibs 3 REQUIRED)

configure_file(wxglterm.json.in wxglterm.json)
configure_file(wxglterm_install.json.in wxglterm_install.json)
configure_file(wxglterm.desktop.in wxglterm.desktop)

ADD_SUBDIRECTORY(src/interface)
ADD_SUBDIRECTORY(src/controller)
ADD_SUBDIRECTORY(src/utils)
ADD_SUBDIRECTORY(src/plugins)
ADD_SUBDIRECTORY(src/app)
ADD_SUBDIRECTORY(src/external_plugins)

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/termdata.stamp
  COMMAND
  ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data ${CMAKE_BINARY_DIR}/data
  COMMAND
  ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/pysrc/libs ${CMAKE_BINARY_DIR}/libs
  COMMENT
  "Copying term data..."
  VERBATIM
  )

# uninstall target
if(NOT TARGET uninstall)
    configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY)

    add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

ADD_CUSTOM_TARGET(copy_term_data ALL DEPENDS ${CMAKE_BINARY_DIR}/termdata.stamp wxglterm)

INSTALL(FILES ${CMAKE_BINARY_DIR}/wxglterm.desktop DESTINATION share/applications)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/data DESTINATION share/wxglterm)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/pysrc/libs DESTINATION share/wxglterm)
INSTALL(FILES ${CMAKE_BINARY_DIR}/wxglterm_install.json DESTINATION share/wxglterm/etc RENAME wxglterm.json)

INCLUDE(CPack)
