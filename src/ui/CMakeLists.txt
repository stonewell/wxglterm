FIND_PACKAGE(wxWidgets 3 REQUIRED
  std
  richtext
  )

FILE(GLOB wxglterm_src
    "*.h"
    "*.cpp"
    )

ADD_DEFINITIONS(-Wall
    )

ADD_EXECUTABLE(wxglterm ${wxglterm_src})

TARGET_COMPILE_DEFINITIONS(wxglterm PRIVATE
    ${wxWidgets_DEFINITIONS}
    )
TARGET_INCLUDE_DIRECTORIES(wxglterm PRIVATE
    ${wxWidgets_INCLUDE_DIRS}
    )
TARGET_LINK_LIBRARIES(wxglterm
	${wxWidgets_LIBRARIES}
	)
