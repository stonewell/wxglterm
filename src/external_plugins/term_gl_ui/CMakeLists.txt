FIND_PACKAGE(glfw3 3.2 REQUIRED)

INCLUDE(ExternalProject)

EXTERNALPROJECT_ADD(
  freetype-gl

  GIT_REPOSITORY "https://github.com/rougier/freetype-gl.git"
  GIT_TAG "master"

  UPDATE_COMMAND ${GIT_EXECUTABLE} pull
  PATCH_COMMAND ""

  CMAKE_ARGS -Dfreetype-gl_BUILD_APIDOC=OFF -Dfreetype-gl_BUILD_DEMOS=OFF -Dfreetype-gl_BUILD_TESTS=OFF -Dfreetype-gl_BUILD_MAKEFONT=OFF

  TEST_COMMAND ""

  INSTALL_COMMAND ""
)

SET(term_gl_ui_src
  term_gl_ui_plugin.cpp
)

ADD_LIBRARY(term_gl_ui SHARED ${term_gl_ui_src})

SET_TARGET_PROPERTIES( term_gl_ui
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)

EXTERNALPROJECT_GET_PROPERTY(freetype-gl source_dir)

SET(freetype-gl_INCLUDE_DIR ${source_dir})

TARGET_INCLUDE_DIRECTORIES(term_gl_ui
    PUBLIC
	$<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
    PRIVATE
	"${CMAKE_CURRENT_SRC_DIR}"
        "${pybind11_INCLUDE_DIRS}"
	"${PYTHON_INCLUDE_DIRS}"
  	"ui"
  	"../../interface"
  	"../../utils"
  	"../../utils/portable_thread/includes"
	"${freetype-gl_INCLUDE_DIR}"
)
