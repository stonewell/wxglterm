FILE(GLOB controller_src
    "*.h"
    "*.cpp"
    )

ADD_LIBRARY(controller STATIC ${controller_src})
ADD_DEPENDENCIES(controller pybind11)

TARGET_INCLUDE_DIRECTORIES(controller PRIVATE
  "../interface"
  "../utils"
)

TARGET_LINK_LIBRARIES(controller PRIVATE
    pybind11::embed
)
