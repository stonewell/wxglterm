FILE(GLOB plugins_src
    "*.h"
    "*.cpp"
    )

ADD_LIBRARY(plugins STATIC ${plugins_src})

TARGET_COMPILE_DEFINITIONS(plugins PRIVATE
    ${wxWidgets_DEFINITIONS}
    )
TARGET_INCLUDE_DIRECTORIES(plugins PRIVATE
    ${wxWidgets_INCLUDE_DIRS}
  "../interface"
  "../utils"
  "../ui"
)
