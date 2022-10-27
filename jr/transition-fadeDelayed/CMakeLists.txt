project(transition-fadeDelayed)

add_library(transition-fadeDelayed MODULE)
# add_library(OBS::obs-textml ALIAS obs-textml)

target_link_libraries(transition-fadeDelayed PRIVATE OBS::libobs)

set_target_properties(transition-fadeDelayed PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "Delayed version of fade transition module")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in transition-fadeDelayed.rc)

target_sources(transition-fadeDelayed PRIVATE transition-fadeDelayed.c transition-fadeDelayed.rc)

target_link_libraries(transition-fadeDelayed)

target_compile_definitions(
  transition-fadeDelayed PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(transition-fadeDelayed)
