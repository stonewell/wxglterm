INCLUDE(ExternalProject)

EXTERNALPROJECT_ADD(
  scintilla

  GIT_REPOSITORY "https://github.com/mirror/scintilla.git"
  GIT_TAG "master"

  PATCH_COMMAND ""
  UPDATE_DISCONNECTED ON
  TEST_COMMAND ""
  INSTALL_COMMAND ""
)

ExternalProject_Add_StepTargets(scintilla update)
ADD_DEPENDENCIES(update_external scintilla-update)

EXTERNALPROJECT_GET_PROPERTY(scintilla SOURCE_DIR BINARY_DIR)

ExternalProject_Add_Step(scintilla pre-configure
   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.scintilla.txt ${SOURCE_DIR}/CMakeLists.txt
   DEPENDEES download
   DEPENDERS configure
   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.scintilla.txt
   BYPRODUCTS ${SOURCE_DIR}/CMakeLists.txt
   )

SET(scintilla_SOURCE_DIR ${SOURCE_DIR}/src)
SET(scintilla_LIBRARIES ${BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}scintilla_cross_platform${CMAKE_STATIC_LIBRARY_SUFFIX})

SET(scintilla_editor_src
  platform.cxx
  scintilla_editor_plugin.cxx
  scintilla_editor_buffer.cxx scintilla_editor_buffer.h
)

ADD_LIBRARY(scintilla_editor SHARED ${scintilla_editor_src})

ADD_DEPENDENCIES(scintilla_editor scintilla)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(no_undefined "-Wl,--no-undefined")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(no_undefined "-Wl,-undefined,error")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(no_undefined "")
endif()

SET_TARGET_PROPERTIES(scintilla_editor
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    CXX_STANDARD 17
    LINK_FLAGS "${no_undefined}"
)

SET(scintilla_INCLUDE_DIR ${SOURCE_DIR}/include
   ${SOURCE_DIR}/src
   ${SOURCE_DIR}/lexlib)

TARGET_INCLUDE_DIRECTORIES(scintilla_editor
    PRIVATE
	"${CMAKE_CURRENT_SRC_DIR}"
	"${CMAKE_CURRENT_BINARY_DIR}"
        "${pybind11_INCLUDE_DIRS}"
	"${PYTHON_INCLUDE_DIRS}"
  	"ui"
  	"../../interface"
  	"../../utils"
  	"../../utils/portable_thread/includes"
	"../../plugins"
	"${scintilla_INCLUDE_DIR}"
)

TARGET_LINK_LIBRARIES(scintilla_editor
  utils
  plugins
  ${pybind11_LIBRARIES}
  ${PYTHON_LIBRARIES}
  "${scintilla_LIBRARIES}"
  )

INSTALL(TARGETS scintilla_editor DESTINATION share/wxglterm/plugins)
