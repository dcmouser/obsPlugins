project(obs-textml)

add_library(obs-textml MODULE)
add_library(OBS::obs-textml ALIAS obs-textml)

target_link_libraries(obs-textml PRIVATE OBS::libobs)

set_target_properties(obs-textml PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "JR OBS GDI+ text module")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in
               obs-textml.rc)

target_sources(obs-textml PRIVATE gdiplus/obs-textml.cpp gdiplus/jrcolorhelper.cpp gdiplus/jrcolorhelper.hpp gdiplus/winklerrgbhsv.cpp gdiplus/winklerrgbhsv.hpp obs-textml.rc)

target_link_libraries(obs-textml PRIVATE gdiplus)

target_compile_definitions(
  obs-textml PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(obs-textml)
