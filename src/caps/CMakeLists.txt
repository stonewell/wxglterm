SET(caps_src
  cap_manager.cpp
  cap_manager.h
  cap_cursor.cpp
  cap_attrs.cpp
  cap_query.cpp
  cap_cell_data.cpp
  cap_scroll_region.cpp
  cap_mode.cpp
  cap_tab.cpp
  cap_window_properties.cpp
  cap_osc_colors.cpp
  cap_osc_selection.cpp
  read_termdata.cpp
  read_termdata.h
  parse_termdata.cpp
  parse_termdata.h
  term_data_param.h
  cap_state.cpp)

ADD_LIBRARY(caps OBJECT
  ${caps_src}
  )

TARGET_INCLUDE_DIRECTORIES(caps PRIVATE
  "${CMAKE_CURRENT_SRC_DIR}"
  "../interface"
  "../utils"
  "${pybind11_INCLUDE_DIRS}"
  "${PYTHON_INCLUDE_DIRS}"
  )
