CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

if (POLICY CMP0048)
CMAKE_POLICY(
    SET CMP0048 NEW
    )
endif()

if (POLICY CMP0058)
CMAKE_POLICY(
    SET CMP0058 NEW
)
endif()

if (POLICY CMP0054)
CMAKE_POLICY(
    SET CMP0054 NEW
)
endif()

if (POLICY CMP0072)
CMAKE_POLICY(
    SET CMP0072 NEW
)
endif()

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
IF (APPLE)
   OPTION(USE_GLFW_HACK_FIX "use glfw hack fix for macos mojave or upper" OFF)
ENDIF(APPLE)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

SET(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

STRING(TIMESTAMP BUILD_OPTIONS "%Y%m%d%H%M%S" UTC)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  ADD_COMPILE_OPTIONS(-Wall -Wextra -Werror -Wno-cast-function-type -Wno-ignored-qualifiers)
  ADD_LINK_OPTIONS(-Wl,--no-undefined)
  ADD_COMPILE_DEFINITIONS(_hypot=hypot)
elseif (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang"))
  ADD_COMPILE_OPTIONS(-Wall -Wextra -Werror -Wno-inconsistent-missing-override)
  ADD_LINK_OPTIONS(-Wl,-undefined,error)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  ADD_COMPILE_OPTIONS(/W4 /WX /EHsc /wd4250 /wd4245 /D_CRT_SECURE_NO_WARNINGS)
endif()

if (MINGW)
  ADD_COMPILE_DEFINITIONS(_UNICODE)
  ADD_COMPILE_DEFINITIONS(UNICODE)
  ADD_COMPILE_OPTIONS(-Wa,-mbig-obj)
  ADD_LINK_OPTIONS(-municode)
endif()

FIND_PACKAGE(PythonInterp 3 REQUIRED)
FIND_PACKAGE(PythonLibs 3 REQUIRED)

if (WIN32)
SET(network_plugin "term_network_win32_console")
SET(network_plugin_shell "cmd.exe")
else()
SET(network_plugin "term_network_use_pty")
SET(network_plugin_shell "/bin/bash")
endif()
configure_file(wxglterm.json.in wxglterm.json)
configure_file(wxglterm_install.json.in wxglterm_install.json)
configure_file(wxglterm.desktop.in wxglterm.desktop)

FIND_PACKAGE(Boost 1.62 REQUIRED)

IF (APPLE)
   EXEC_PROGRAM(uname ARGS -v  OUTPUT_VARIABLE DARWIN_VERSION)
   STRING(REGEX MATCH "[0-9]+" DARWIN_VERSION ${DARWIN_VERSION})
   MESSAGE(STATUS "DARWIN_VERSION=${DARWIN_VERSION}")
   IF (DARWIN_VERSION GREATER_EQUAL 18)
      SET(USE_GLFW_HACK_FIX ON)
   ENDIF (DARWIN_VERSION GREATER_EQUAL 18)
ENDIF(APPLE)

IF (USE_GLFW_HACK_FIX)
   ADD_SUBDIRECTORY(glfw_fix)
   SET(GLFW_HACK_FIX_LIB glfw_fix "-framework Foundation")
   SET(GLFW_HACK_FIX_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/glfw_fix")
   MESSAGE(STATUS "add glfw hack fix for macos > 10.14")
   MESSAGE(STATUS "include dir:${GLFW_HACK_FIX_INCLUDE_DIR}")
   ADD_DEFINITIONS(-DGLFW_HACK_FIX -DGL_SILENCE_DEPRECATION)
ENDIF()

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/third_party/pybind11")
    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

ADD_CUSTOM_TARGET(update_external)

ADD_SUBDIRECTORY(third_party/pybind11)
ADD_SUBDIRECTORY(src/interface)
ADD_SUBDIRECTORY(src/controller)
ADD_SUBDIRECTORY(src/utils)
ADD_SUBDIRECTORY(src/caps)
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
