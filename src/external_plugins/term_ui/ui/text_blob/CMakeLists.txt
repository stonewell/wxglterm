SET(text_blob_src
    text_blob.h text_blob.cxx
    font_manager.cxx font_manager.h
    font_impl.h font_impl.cxx
    font.h
    err_msg.cxx err_msg.h
    char_width.h char_width.cxx
)

IF (APPLE)
    LIST(APPEND text_blob_src
        text_blob_generic.cxx)
    SET(EXTRA_INCLUDE "")

ELSE()
    LIST(APPEND text_blob_src
        text_blob_cairo.cxx)
    SET(EXTRA_INCLUDE ${Cairo_INCLUDE_DIR})
ENDIF()

ADD_LIBRARY(text_blob OBJECT
  ${text_blob_src}
  )

TARGET_COMPILE_DEFINITIONS(text_blob PRIVATE
    ${wxWidgets_DEFINITIONS}
    )

TARGET_INCLUDE_DIRECTORIES(text_blob PRIVATE
   ${wxWidgets_INCLUDE_DIRS}
   ${FREETYPE_INCLUDE_DIRS}
   ${EXTRA_INCLUDE}
)
