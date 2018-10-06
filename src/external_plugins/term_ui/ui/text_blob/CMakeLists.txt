SET(text_blob_src
    text_blob.h text_blob.cxx
    text_blob_cairo.cxx
    font_manager.cxx font_manager.h
    font_impl.h font_impl.cxx
    font.h
    err_msg.cxx err_msg.h
    char_width.h char_width.cxx
)

ADD_LIBRARY(text_blob OBJECT
  ${text_blob_src}
  )

TARGET_COMPILE_DEFINITIONS(text_blob PRIVATE
    ${wxWidgets_DEFINITIONS}
    )

TARGET_INCLUDE_DIRECTORIES(text_blob PRIVATE
   ${wxWidgets_INCLUDE_DIRS}
   ${FREETYPE_INCLUDE_DIRS}
   "${Cairo_INCLUDE_DIR}"
)
