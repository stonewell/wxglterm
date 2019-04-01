FILE(GLOB wxglterm_interface_src
    "*.h"
    "*.cpp"
    )

ADD_LIBRARY(wxglterm_interface_static ${wxglterm_interface_src})

TARGET_LINK_LIBRARIES(wxglterm_interface_static PRIVATE
    pybind11::embed
    )
