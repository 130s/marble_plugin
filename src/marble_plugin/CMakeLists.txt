find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)
find_package(Marble REQUIRED)

include(${QT_USE_FILE})

set(marble_plugin_SRCS
  marble_plugin.cpp
)

set(marble_plugin_HDRS
  marble_plugin.h
)

set(marble_plugin_UIS
  marble_plugin.ui
)

set(marble_plugin_INCLUDE_DIRECTORIES
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/build/src
)

qt4_wrap_cpp(marble_plugin_MOCS ${marble_plugin_HDRS})
qt4_wrap_ui(marble_plugin_UIS_H ${marble_plugin_UIS})

rosbuild_include(pluginlib hide_plugin_symbols)
pluginlib_hide_plugin_symbols(marble_plugin MarblePlugin)

include_directories(marble_plugin ${marble_plugin_INCLUDE_DIRECTORIES} ${MARBLE_INCLUDE_DIR} )
rosbuild_add_library(marble_plugin ${marble_plugin_SRCS} ${marble_plugin_MOCS} ${marble_plugin_UIS_H})
target_link_libraries(marble_plugin ${QT_LIBRARIES} ${MARBLE_LIBRARIES})

