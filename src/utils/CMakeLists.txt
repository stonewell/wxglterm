SET(utils_src
    app_config_impl.cpp  load_module.cpp  string_utils.cpp
    app_config_impl.h  load_module.h  string_utils.h
    char_width.cpp char_width.h
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
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  ${CMAKE_CURRENT_BINARY_DIR}
  "../interface"
)
