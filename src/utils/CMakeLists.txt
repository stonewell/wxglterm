Find_Package(Threads REQUIRED)

SET(utils_src
    app_config_impl.cpp  load_module.cpp  string_utils.cpp
    app_config_impl.h  load_module.h  string_utils.h
    char_width.cpp char_width.h
    base64.h
    smart_object_pool.h
    buffer_roller.h buffer_roller.cpp
    utf8_util.h utf8_util.cpp
    buffer_line_mapper.h buffer_line_mapper.cpp
    )

IF (WIN32)
  SET(utils_platform_src
    platform_specific/windows/load_module_win.cpp
    )
ELSE()
  SET(utils_platform_src
    platform_specific/unix_like/load_module_unix_like.cpp
    )
ENDIF()

ADD_CUSTOM_COMMAND(OUTPUT app_config.inc
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/pystmt_generator/pystmt_generator.py -i ${CMAKE_CURRENT_SOURCE_DIR}/app_config.py -o app_config.inc
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/app_config.py ${CMAKE_CURRENT_SOURCE_DIR}/../../utils/pystmt_generator/pystmt_generator.py
  )

ADD_LIBRARY(utils STATIC
  ${utils_src}
  ${utils_platform_src}
  portable_thread/src/PortableThread.cpp
  app_config.inc
  )

TARGET_INCLUDE_DIRECTORIES(utils PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "${CMAKE_CURRENT_SRC_DIR}/portable_thread/include"
  ${CMAKE_CURRENT_BINARY_DIR}
  "../interface"
)

TARGET_LINK_LIBRARIES(utils PRIVATE
    pybind11::embed
    )
TARGET_LINK_LIBRARIES(utils PUBLIC
    Threads::Threads
    )
