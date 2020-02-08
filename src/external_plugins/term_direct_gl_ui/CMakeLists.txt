FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(Freetype REQUIRED)
FIND_PACKAGE(glfw3 3.2 REQUIRED)

INCLUDE(ExternalProject)

SET(FREETYPE_GL_UPDATE git pull)

EXTERNALPROJECT_ADD(
  freetype-direct-gl

  GIT_REPOSITORY "https://github.com/stonewell/freetype-direct-gl.git"
  GIT_TAG "master"

  PATCH_COMMAND ""
  UPDATE_DISCONNECTED ON

  CMAKE_ARGS -DCMAKE_C_FLAGS=-fPIC -DOpenGL_GL_PREFERENCE=GLVND

  TEST_COMMAND ""

  INSTALL_COMMAND ""
)

ExternalProject_Add_StepTargets(freetype-direct-gl update)
ADD_DEPENDENCIES(update_external freetype-direct-gl-update)

EXTERNALPROJECT_GET_PROPERTY(freetype-direct-gl SOURCE_DIR BINARY_DIR)

SET(term_direct_gl_ui_src
  term_direct_gl_ui_plugin.cpp
  term_direct_gl_ui.cpp term_direct_gl_ui.h
  default_term_window.h default_term_window.cpp
  default_term_window_draw.cpp default_term_window_key.cpp
  default_term_window_mouse.cpp
  freetype_gl.cpp freetype_gl.h
)

ADD_LIBRARY(term_direct_gl_ui SHARED ${term_direct_gl_ui_src}
		       )

ADD_DEPENDENCIES(term_direct_gl_ui freetype-direct-gl)

SET_TARGET_PROPERTIES( term_direct_gl_ui
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)


FIND_PACKAGE(FontConfig REQUIRED)
IF(NOT APPLE)
  FIND_PACKAGE(GLEW REQUIRED)
  IF(${CMAKE_VERSION} VERSION_LESS "3.15.0")
    SET(GLEW_LIB ${GLEW_LIBRARY})
  ELSE()
    SET(GLEW_LIB GLEW::GLEW)
  ENDIF()
ENDIF()

FIND_PACKAGE(GLM)

SET(freetype-direct-gl_INCLUDE_DIR ${SOURCE_DIR})
SET(freetype-direct-gl_LIBRARIES ${BINARY_DIR}/src/${CMAKE_STATIC_LIBRARY_PREFIX}freetype_direct_gl${CMAKE_STATIC_LIBRARY_SUFFIX})

TARGET_INCLUDE_DIRECTORIES(term_direct_gl_ui
    PRIVATE
	"${CMAKE_CURRENT_SRC_DIR}"
	"${CMAKE_CURRENT_BINARY_DIR}"
 	"ui"
  	"../../interface"
  	"../../utils"
  	"../../utils/portable_thread/includes"
	"../../plugins"
	"${freetype-direct-gl_INCLUDE_DIR}/src/font"
	"${freetype-direct-gl_INCLUDE_DIR}/src/text"
	"${freetype-direct-gl_INCLUDE_DIR}/src/render"
	"${freetype-direct-gl_INCLUDE_DIR}/src/viewport"
	"${FONTCONFIG_INCLUDE_DIR}"
	"${freetype-direct-gl_INCLUDE_DIR}/windows/freetype"
	$<TARGET_PROPERTY:glfw,INTERFACE_INCLUDE_DIRECTORIES>
	${OPENGL_INCLUDE_DIR}
	"${GLFW_HACK_FIX_INCLUDE_DIR}"
)

TARGET_LINK_LIBRARIES(term_direct_gl_ui PRIVATE
  utils
  plugins
  glfw
  pybind11::embed
  ${OPENGL_LIBRARY}
  ${FREETYPE_LIBRARIES}
  ${freetype-direct-gl_LIBRARIES}
  ${FONTCONFIG_LIBRARY}
  ${GLEW_LIB}
  ${GLFW_HACK_FIX_LIB}
  )

INSTALL(TARGETS term_direct_gl_ui DESTINATION share/wxglterm/plugins)
