#
# Find libcairo
#

# This module defines
# Cairo_FOUND
# Cairo_INCLUDE_DIR
# Cairo_LIBRARY

set(Cairo_FOUND 0)

find_path(Cairo_INCLUDE_DIR cairo.h PATH_SUFFIXES cairo )
find_library(Cairo_LIBRARY cairo)

if(Cairo_INCLUDE_DIR)
  if(Cairo_LIBRARY)
    set(Cairo_FOUND 1)
  endif()
endif()

mark_as_advanced(Cairo_FOUND Cairo_INCLUDE_DIR Cairo_LIBRARY)
