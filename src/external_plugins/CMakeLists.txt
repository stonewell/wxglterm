IF (WIN32)
ADD_SUBDIRECTORY(term_network_win32_console)
ELSE()
ADD_SUBDIRECTORY(term_network_pty)
ENDIF()

ADD_SUBDIRECTORY(term_data_handler)

IF (BUILD_WXWIDGETS_UI)
ADD_SUBDIRECTORY(term_ui)
ENDIF()

IF (BUILD_OPENGL_UI)
ADD_SUBDIRECTORY(term_direct_gl_ui)
ADD_SUBDIRECTORY(term_gl_ui)
ENDIF()

ADD_SUBDIRECTORY(term_input_handler)
ADD_SUBDIRECTORY(term_buffer)
ADD_SUBDIRECTORY(term_shmem_buffer)

IF (BUILD_SCINTILLA_EDITOR)
ADD_SUBDIRECTORY(scintilla_editor)
ADD_SUBDIRECTORY(scintilla_editor_input_handler)
ADD_SUBDIRECTORY(scintilla_editor_data_handler)
ENDIF()
