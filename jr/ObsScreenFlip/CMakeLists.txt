project(ObsScreenFlip)

add_library(ObsScreenFlip MODULE)

target_link_libraries(ObsScreenFlip PRIVATE OBS::libobs OBS::frontend-api)

set_target_properties(ObsScreenFlip PROPERTIES FOLDER "plugins/jr")

set(MODULE_DESCRIPTION "OBS Preview Window Streaming Mirror Screen Flip")
configure_file(${CMAKE_SOURCE_DIR}/cmake/bundle/windows/obs-module.rc.in ObsScreenFlip.rc)

target_sources(ObsScreenFlip PRIVATE ObsScreenFlip.rc src/pluginRender.cpp src/pluginCodeSetttings.cpp src/obsHelpers.cpp src/jrPluginMainRenderHook.cpp src/jrPlugin.cpp src/jrfuncs.cpp)

target_link_libraries(ObsScreenFlip)

target_compile_definitions(
  ObsScreenFlip PRIVATE UNICODE _UNICODE _CRT_SECURE_NO_WARNINGS
                   _CRT_NONSTDC_NO_WARNINGS)

setup_plugin_target(ObsScreenFlip)
