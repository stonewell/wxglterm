SET(text_blob_src
    text_blob.h text_blob.cxx
    text_blob_cairo.cxx
)

ADD_LIBRARY(text_blob OBJECT
  ${text_blob_src}
  )

TARGET_COMPILE_DEFINITIONS(text_blob PRIVATE
    ${wxWidgets_DEFINITIONS}
    )

TARGET_INCLUDE_DIRECTORIES(text_blob PRIVATE
   ${wxWidgets_INCLUDE_DIRS}
   "${Cairo_INCLUDE_DIR}"
)
