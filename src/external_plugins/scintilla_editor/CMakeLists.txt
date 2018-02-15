INCLUDE(ExternalProject)

EXTERNALPROJECT_ADD(
  scintilla

  GIT_REPOSITORY "https://github.com/mirror/scintilla.git"
  GIT_TAG "master"

  PATCH_COMMAND ""
  UPDATE_COMMAND ""
  BUILD_COMMAND ""
  TEST_COMMAND ""
  INSTALL_COMMAND ""
  CONFIGURE_COMMAND ""
)

EXTERNALPROJECT_GET_PROPERTY(scintilla SOURCE_DIR BINARY_DIR)

SET(scintilla_SOURCE_DIR ${SOURCE_DIR}/src)

SET(scintilla_editor_src
  platform.cxx
  scintilla_editor_plugin.cxx
  scintilla_editor.cxx scintilla_editor.h
  ${CMAKE_CURRENT_BINARY_DIR}/Document.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/CellBuffer.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/PerLine.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/LineMarker.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/Decoration.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/RESearch.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/CharClassify.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/RunStyles.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/XPM.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/UniConversion.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/CharacterCategory.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/Editor.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/EditModel.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/EditView.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/ViewStyle.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/ScintillaBase.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/Catalogue.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/CallTip.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/CaseConvert.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/CaseFolder.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/CaseFolder.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/AutoComplete.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/CharacterSet.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/Selection.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/RunStyles.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/Style.cxx
  )

ADD_CUSTOM_COMMAND(OUTPUT Document.cxx
			  CellBuffer.cxx
			  PerLine.cxx
			  LineMarker.cxx
			  Decoration.cxx
			  RESearch.cxx
			  CharClassify.cxx
			  RunStyles.cxx
			  XPM.cxx
			  UniConversion.cxx
			  CharacterCategory.cxx
			  Editor.cxx
			  EditModel.cxx
			  EditView.cxx
			  ViewStyle.cxx
			  ScintillaBase.cxx
			  Catalogue.cxx
			  CallTip.cxx
			  CaseConvert.cxx
			  CaseFolder.cxx
			  AutoComplete.cxx
			  CharacterSet.cxx
			  Selection.cxx
			  RunStyles.cxx
			  Style.cxx
			  PositionCache.cxx
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/ViewStyle.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/RunStyles.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/Style.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/Selection.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/AutoComplete.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/CaseConvert.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/CaseFolder.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/Catalogue.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/CallTip.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/ScintillaBase.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/Document.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/Editor.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/EditModel.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/EditView.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/CellBuffer.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/PerLine.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/LineMarker.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/Decoration.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/RESearch.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/XPM.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/CharClassify.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/RunStyles.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${scintilla_SOURCE_DIR}/UniConversion.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${SOURCE_DIR}/lexlib/CharacterCategory.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND
  ${CMAKE_COMMAND} -E copy_if_different ${SOURCE_DIR}/lexlib/CharacterSet.cxx ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT
  "Copying scintilla source code..."
  VERBATIM
  )

ADD_LIBRARY(scintilla_editor SHARED ${scintilla_editor_src}
		       )

ADD_DEPENDENCIES(scintilla_editor scintilla)

SET_TARGET_PROPERTIES(scintilla_editor
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    CXX_STANDARD 17
    LINK_FLAGS "-Wl,--no-undefined"
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
  )

INSTALL(TARGETS scintilla_editor DESTINATION share/wxglterm/plugins)
