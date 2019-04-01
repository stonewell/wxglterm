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

SET(scintilla_DIR ${SOURCE_DIR})
SET(scintilla_SOURCE_DIR ${SOURCE_DIR}/src)
SET(scintilla_LIBRARIES ${BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}scintilla_cross_platform${CMAKE_STATIC_LIBRARY_SUFFIX})

EXTERNALPROJECT_ADD(
  scite

  GIT_REPOSITORY "https://github.com/mirror/scite.git"
  GIT_TAG "master"

  PATCH_COMMAND ""
  UPDATE_DISCONNECTED ON
  TEST_COMMAND ""
  INSTALL_COMMAND ""
  CMAKE_ARGS "-DSCINTILLA_DIR=${scintilla_DIR}"
  )

ExternalProject_Add_StepTargets(scite update)
ADD_DEPENDENCIES(update_external scite-update)

EXTERNALPROJECT_GET_PROPERTY(scite SOURCE_DIR BINARY_DIR)

ExternalProject_Add_Step(scite pre-configure
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.scite.txt ${SOURCE_DIR}/CMakeLists.txt
  DEPENDEES download
  DEPENDERS configure
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.scite.txt
  BYPRODUCTS ${SOURCE_DIR}/CMakeLists.txt
  )

ExternalProject_Add_Step(scite copy-properties
  COMMAND COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_install_scite_properties.cmake
  DEPENDEES build
  DEPENDERS test
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/cmake_install_scite_properties.cmake
  )

SET(scite_DIR ${SOURCE_DIR})
SET(scite_SOURCE_DIR ${SOURCE_DIR}/src)
SET(scite_LIBRARIES ${BINARY_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}scite_cross_platform${CMAKE_STATIC_LIBRARY_SUFFIX})

configure_file(cmake_install_scite_properties.cmake.in
  cmake_install_scite_properties.cmake
  IMMEDIATE @ONLY)

SET(scintilla_editor_src
  platform.cxx
  window.cxx
  surface.cxx
  scintilla_editor_plugin.cxx
  scintilla_editor_buffer.cxx scintilla_editor_buffer.h
  scintilla_editor.cxx scintilla_editor.h
  scintilla_editor_line.cxx scintilla_editor_line.h
  scintilla_editor_cell.cxx scintilla_editor_cell.h
  scite_impl.cxx scite_impl.h
  )


ADD_LIBRARY(scintilla_editor SHARED ${scintilla_editor_src})

ADD_DEPENDENCIES(scintilla_editor scintilla scite)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(PLATFORM_DEF "-DGTK -D__unix__ ")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  set(PLATFORM_DEF "-D__APPLE__ -D__unix__ ")
endif()

TARGET_COMPILE_DEFINITIONS(scintilla_editor PRIVATE
  "${PLATFORM_DEF} -g"
  )

SET_TARGET_PROPERTIES(scintilla_editor
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
  CXX_STANDARD 17
  )

SET(scintilla_INCLUDE_DIR ${scintilla_DIR}/include
  ${scintilla_DIR}/src
  ${scintilla_DIR}/lexlib
  )

TARGET_INCLUDE_DIRECTORIES(scintilla_editor
  PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
 "ui"
  "../../interface"
  "../../utils"
  "../../utils/portable_thread/includes"
  "../../plugins"
  "${scintilla_INCLUDE_DIR}"
  "${scite_DIR}/src"
  )

TARGET_LINK_LIBRARIES(scintilla_editor
  utils
  plugins
  "${scintilla_LIBRARIES}"
  "${scite_LIBRARIES}"
  )

INSTALL(TARGETS scintilla_editor DESTINATION share/wxglterm/plugins)
