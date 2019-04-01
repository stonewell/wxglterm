ADD_SUBDIRECTORY(text_blob)

ADD_LIBRARY(wxglterm_ui
    scope_locker.h
    draw_pane.h draw_pane.cpp
    main_dlg.cpp        main_dlg.h
    draw_pane_key.cpp draw_pane_draw.cpp
    draw_pane_mouse.cpp
    $<TARGET_OBJECTS:text_blob>
)

FIND_LIBRARY(APPLICATION_SERVICES_FRAMEWORK ApplicationServices REQUIRED)

TARGET_COMPILE_DEFINITIONS(wxglterm_ui PRIVATE
    ${wxWidgets_DEFINITIONS}
    )

TARGET_INCLUDE_DIRECTORIES(wxglterm_ui PRIVATE
    "${wxWidgets_INCLUDE_DIRS}"
    ${FREETYPE_INCLUDE_DIRS}
    "../../../controller"
    "../../../interface"
    "../../../utils"
    "text_blob"
    )

TARGET_LINK_LIBRARIES(wxglterm_ui PUBLIC
    pybind11::embed
    )
